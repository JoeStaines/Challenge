#define BOOST_ALL_NO_LIB

#include <boost/interprocess/containers/vector.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <sys/wait.h>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>

#include <vector>

#include "DataPointsProducer.h"
#include "DataPointsConsumer.h"
#include "HDF5HandlerBase.h"
#include "H5Cpp.h"

using namespace boost::interprocess;
//Alias an STL-like allocator of ints that allocates ints from the segment
typedef allocator<int, managed_shared_memory::segment_manager>
  ShmemAllocator;

//Alias a vector that uses the previous STL-like allocator
typedef vector<int, ShmemAllocator> MyVector;

//Alias of circular buffer
typedef boost::circular_buffer<int, ShmemAllocator> MyCircularBuffer;

// Struct for mutex and cond variable
struct sync_items {

    sync_items ()
        : finished(false)
    {}

    // Mutex to control access to circular buffer
    interprocess_mutex      mutex;

    // Cond var for the consumer to wait when circular buffer is empty
    interprocess_condition  cond_empty;

    // Finish producing/consuming, break any loops
    bool                    finished;
};


int main (int argc, char* argv[])
{
    srand(time(NULL));


    std::string pointsStr =     "MySharedMemory";
    std::string syncStr =       "SyncMemory";
    std::string bufferStr =     "MyCircularBuffer";
    std::string syncItemsStr =  "SyncItems";

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        char * remove_str;

        shm_remove(char* str) { remove_str = str; shared_memory_object::remove(remove_str); }
        ~shm_remove(){ shared_memory_object::remove(remove_str); }
    } remover(const_cast<char*>(pointsStr.c_str()));

    // Need 10^6 data points every 1s so 10*10^6 space needs to be allocated
    // then after 10s it will start to circle back and overwrite.
    // Plus a small buffer for any extra space that may be allocated
    // such as the container object
    const int capacity =    10000000;
    const int data_count =  1000000;
    const int buffer =      1024;

    //A managed shared memory where we can construct objects
    //associated with a c-string
    std::cout << "Created a whole lot of shared memory..." << std::endl;
    managed_shared_memory segment(create_only,
                                 pointsStr.c_str(),  //segment name
                                 capacity*sizeof(int)+buffer);


    //Initialize the STL-like allocator
    const ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<MyCircularBuffer> circ_buffer = segment.construct<MyCircularBuffer>(bufferStr.c_str()) (alloc_inst);
    circ_buffer->set_capacity(capacity);


    //Remover for the syncronisation items
    struct shm_sync_remove
    {
        char * remove_str;

        shm_sync_remove(char* str) {remove_str = str; shared_memory_object::remove(remove_str); }
        ~shm_sync_remove(){ shared_memory_object::remove(remove_str); }
    } sync_remover(const_cast<char*>(syncStr.c_str()));

    managed_shared_memory sync_segment  (create_only,
                                        syncStr.c_str(),
                                        1024);

    sync_segment.construct<sync_items>(syncItemsStr.c_str())();

    int poisonPill = -100; // For this program this val shouldn't be a problem
    int status = 0;
    pid_t cpid;
    pid_t pid = fork();

    if (pid == 0) { // Child process

        std::cout << "child enter consuming func" << std::endl;

        bool finished = false;

        try {
            HDF5HandlerBase hdf5_handler("hdf5_test.h5", "dset");

            DataPointsConsumer consumer(pointsStr, syncStr);

            while(!finished) {
                std::vector<int> data;
                std::cout << "Before consume" << std::endl;
                data = consumer.consume_to_vector(100000, poisonPill);

                if (data.back() == poisonPill) {
                    finished = true;
                    data.pop_back();
                }

                hdf5_handler.save(data);
                std::cout << "After save" << std::endl;
            }

        } catch (FileIException &error) {
            error.printError();
            return -1;
        } catch (DataSetIException error) {
            error.printError();
            return -1;
        } catch(DataSpaceIException error) {
            error.printError();
            return -1;
        }

    } else if (pid > 0) { // Parent process
        std::cout << "parent enter producing func" << std::endl;
        DataPointsProducer producer(pointsStr, syncStr, bufferStr, syncItemsStr);

        for (int i = 0; i < 5; ++i) {
            producer.produce(data_count);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        producer.finish(poisonPill);
        std::cout << "Parent added poison pill" << std::endl;

        if ((cpid = wait(&status) == pid))
            std::cout << "Child returned" << std::endl;
        //std::cout << "HDF5 return val " << hdf5_handler.extendable_file() << std::endl;

    } else { // Fork failed
        std::cout << "Fork() failed. Aborting" << std::endl;
        return 1;
    }

    //std::cout << "Press Enter" << std::endl;
    //std::cin.ignore();
    // Destroy circular buffer  and sync items from memory
    segment.destroy<MyCircularBuffer>("MyCircularBuffer");
    sync_segment.destroy<sync_items>("SyncItems");

    return 0;
}

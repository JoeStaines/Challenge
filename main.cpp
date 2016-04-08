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

#include "DataPointsProducer.h"
#include "DataPointsConsumer.h"

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

    /*** Note: start refactoring producer/consumer to their own classes ***/

    std::string pointsStr =     "MySharedMemory";
    std::string syncStr =       "SyncMemory";

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
    } remover;

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

    offset_ptr<MyCircularBuffer> circ_buffer = segment.construct<MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circ_buffer->set_capacity(capacity);


    //Remover for the syncronisation items
    struct shm_sync_remove
    {
        shm_sync_remove() { shared_memory_object::remove("SyncMemory"); }
        ~shm_sync_remove(){ shared_memory_object::remove("SyncMemory"); }
    } sync_remover;

    managed_shared_memory sync_segment  (create_only,
                                        syncStr.c_str(),
                                        1024);

    sync_segment.construct<sync_items>("SyncItems")();

    int status = 0;
    pid_t child_proc = fork();

    if (child_proc == 0) { // Child process
        std::cout << "child enter consuming func" << std::endl;
        DataPointsConsumer consumer(pointsStr, syncStr);
        consumer.take_points_from_buffer();

    } else if (child_proc > 0) { // Parent process
        std::cout << "parent enter producing func" << std::endl;
        DataPointsProducer producer(pointsStr, syncStr, data_count);
        producer.add_points_to_buffer();

    } else { // Fork failed
        std::cout << "Fork() failed. Aborting" << std::endl;
        return 1;
    }

    // Destroy circular buffer  and sync items from memory
    segment.destroy<MyCircularBuffer>("MyCircularBuffer");
    sync_segment.destroy<sync_items>("SyncItems");

    return 0;
}

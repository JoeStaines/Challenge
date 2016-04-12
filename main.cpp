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
#include "DataPointsModifier.h"
#include "SharedMemoryData.h"
#include "HDF5HandlerBase.h"
#include "H5Cpp.h"

using namespace boost::interprocess;

void single_producer_consumer(shared_data_strings dataStrings, int dataCount)
{
    int poisonPill = -100; // For this program this val shouldn't be a problem
    int status = 0;
    pid_t cpid;
    pid_t pid = fork();

    if (pid == 0) { // Child process

        std::cout << "child enter consuming func" << std::endl;

        bool finished = false;

        try {
            HDF5HandlerBase hdf5_handler("hdf5_test.h5", "dset");

            DataPointsConsumer consumer(dataStrings);

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
            exit(-1);
        } catch (DataSetIException error) {
            error.printError();
            exit(-1);
        } catch(DataSpaceIException error) {
            error.printError();
            exit(-1);
        }

    } else if (pid > 0) { // Parent process
        std::cout << "parent enter producing func" << std::endl;
        DataPointsProducer producer(dataStrings);

        for (int i = 0; i < 5; ++i) {
            producer.produce(dataCount);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        producer.finish(poisonPill);
        std::cout << "Parent added poison pill" << std::endl;

        if ((cpid = wait(&status) == pid))
            std::cout << "Child returned" << std::endl;
        //std::cout << "HDF5 return val " << hdf5_handler.extendable_file() << std::endl;

    } else { // Fork failed
        std::cout << "Fork() failed. Aborting" << std::endl;
        exit(1);
    }
}

void modify_producer_child(shared_data_strings oldBufferStrings, shared_data_strings newBufferStrings)
{
    int poisonPill = -100; // For this program this val shouldn't be a problem
    int status = 0;
    pid_t cpid;
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        bool finished = false;

        try {
            HDF5HandlerBase hdf5Handler("hdf5_modify.h5", "dset");
            DataPointsConsumer consumer(newBufferStrings);

            while(!finished) {
                std::vector<int> data;
                data = consumer.consume_to_vector(100000, poisonPill);

                // Read poison pill, this is the last bit of data
                if (data.back() == poisonPill) {
                    finished = true;
                    data.pop_back();
                }
                hdf5Handler.save(data);
                std::cout << "After modify save" << std::endl;
            }
        } catch (FileIException &error) {
            error.printError();
            exit(-1);
        } catch (DataSetIException error) {
            error.printError();
            exit(-1);
        } catch(DataSpaceIException error) {
            error.printError();
            exit(-1);
        }
    } else if (pid > 0) {
        // Parent process

        // Modify data from old buffer
        DataPointsModifier modifier;
        modifier.set_read_buffer(oldBufferStrings);
        modifier.set_write_buffer(newBufferStrings);
        modifier.modify(poisonPill);

        if ((cpid = wait(&status) == pid))
            std::cout << "Child returned" << std::endl;

    } else {
        std::cout << "fork() failed. Aborting" << std::endl;
        exit(1);
    }

}

void modify_producer(shared_data_strings oldBufferStrings)
{

    // Create new memory segment to modify to
    shared_data_strings newBufferStrings = {"NewSharedMemory", "NewSync", "NewCircBuffer", "NewItems"};

    // Remove shared memory segment if it exists
    shm_remove remover(const_cast<char*>(newBufferStrings.bufferMemory.c_str() ));
    shm_remove sync_remover(const_cast<char*>(newBufferStrings.syncMemory.c_str() ));

    const int capacity =    10000000;
    const int dataCount =  1000000;
    const int buffer =      1024;

    managed_shared_memory newSegment(create_only,
                                 newBufferStrings.bufferMemory.c_str(),  //segment name
                                 capacity*sizeof(int)+buffer);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (newSegment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = newSegment.construct<shmem_data::MyCircularBuffer>(newBufferStrings.bufferObject.c_str()) (alloc_inst);
    circBuffer->set_capacity(capacity);

    managed_shared_memory newSyncSegment  (create_only,
                                        newBufferStrings.syncMemory.c_str(),
                                        1024);

    newSyncSegment.construct<shmem_data::sync_items>(newBufferStrings.syncObject.c_str() )();

    int poisonPill = -100; // For this program this val shouldn't be a problem
    int status = 0;
    pid_t cpid;
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        modify_producer_child(oldBufferStrings, newBufferStrings);

    } else if (pid > 0) {
        // Parent process
        DataPointsProducer producer(oldBufferStrings);

        for (int i = 0; i < 5; ++i) {
            producer.produce(dataCount);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        producer.finish(poisonPill);
        std::cout << "Parent added poison pill" << std::endl;

        if ((cpid = wait(&status) == pid))
            std::cout << "Child returned" << std::endl;
    } else {
        std::cout << "fork() failed. Aborting." << std::endl;
        exit(1);
    }
}


int main (int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: NanoporeChallenge [mode]" << std::endl;
        return 1;
    }
    std::string mode(argv[1]);

    srand(time(NULL));

    shared_data_strings dataStrings = {"MySharedMemory", "SyncMemory", "MyCircularBuffer", "SyncItems"};

    // Remove shared memory segment if it exists
    shm_remove remover(const_cast<char*>(dataStrings.bufferMemory.c_str()));
    shm_remove sync_remover(const_cast<char*>(dataStrings.syncMemory.c_str()));

    // Need 10^6 data points every 1s so 10*10^6 space needs to be allocated
    // then after 10s it will start to circle back and overwrite.
    // Plus a small buffer for any extra space that may be allocated
    // such as the container object
    const int capacity =    10000000;
    const int dataCount =  1000000;
    const int buffer =      1024;

    //A managed shared memory where we can construct objects
    //associated with a c-string
    std::cout << "Created a whole lot of shared memory..." << std::endl;
    managed_shared_memory segment(create_only,
                                 dataStrings.bufferMemory.c_str(),  //segment name
                                 capacity*sizeof(int)+buffer);


    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.construct<shmem_data::MyCircularBuffer>(dataStrings.bufferObject.c_str() ) (alloc_inst);
    circ_buffer->set_capacity(capacity);

    managed_shared_memory sync_segment  (create_only,
                                        dataStrings.syncMemory.c_str(),
                                        1024);

    sync_segment.construct<shmem_data::sync_items>(dataStrings.syncObject.c_str())();

    if (mode == "1") {
        single_producer_consumer(dataStrings, dataCount);
    } else if (mode == "2") {
        modify_producer(dataStrings);
    } else {
        std::cout << "Invalid argument. Aborting" << std::endl;
        exit(1);
    }

    //std::cout << "Press Enter" << std::endl;
    //std::cin.ignore();
    // Destroy circular buffer  and sync items from memory
    segment.destroy<shmem_data::MyCircularBuffer>("MyCircularBuffer");
    sync_segment.destroy<shmem_data::sync_items>("SyncItems");

    return 0;
}

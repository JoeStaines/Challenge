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

void add_points_to_buffer() {
    // Open shared memory segment
    managed_shared_memory segment(open_only, "MySharedMemory");
    managed_shared_memory sync_segment(open_only, "SyncMemory");

    // Open circular buffer from shared memory
    offset_ptr<MyCircularBuffer> circ_buffer = segment.find<MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<sync_items> sync_data = sync_segment.find<sync_items>("SyncItems").first;

    for (int j = 0; j < 5; ++j) {

        // Fill buffer with random ints
        // (increase scale later)
        for (int i = 0; i < 100; ++i) {
            std::this_thread::sleep_for (std::chrono::milliseconds(10));
            scoped_lock<interprocess_mutex> lock (sync_data->mutex);
            int num = rand() % 10;
            std::cout << "Process 1 post data " << num << std::endl;
            circ_buffer->push_back(num);
            sync_data->cond_empty.notify_one(); // Notify that circular buffer not empty
        }

        std::cout << "Process 1 sleep..." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(1));

    }

    sync_data->finished = true;
    sync_data->cond_empty.notify_one(); // Use this to wake up consumer last time in case it's stuck waiting
}

void take_points_from_buffer() {
    // Open shared memory segment
    managed_shared_memory segment(open_only, "MySharedMemory");
    managed_shared_memory sync_segment(open_only, "SyncMemory");

    // Open circular buffer  and sync data from shared memory
    offset_ptr<MyCircularBuffer> circ_buffer = segment.find<MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<sync_items> sync_data = sync_segment.find<sync_items>("SyncItems").first;

    while(!sync_data->finished) {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        if (circ_buffer->empty()) {
            std::cout << "Process 2 buffer empty, waiting..." << std::endl;
            sync_data->cond_empty.wait(lock);
            std::cout << "Process 2 waking up" << std::endl;

            // Use continue so that consumer skips taking out of the buffer
            // and checks the loop condition again + empty condition.
            continue;
        }

        std::cout << "Process 2 taking data " << circ_buffer->front() << std::endl;
        circ_buffer->pop_front();
    }
}

int main (int argc, char* argv[])
{
    srand(time(NULL));

    /*** Note: start refactoring producer/consumer to their own classes ***/

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
    } remover;

    const int capacity = 100000000;
    const int buffer = 1024;

    //A managed shared memory where we can construct objects
    //associated with a c-string
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 65535);


    //Initialize the STL-like allocator
    const ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<MyCircularBuffer> circ_buffer = segment.construct<MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circ_buffer->set_capacity(10000);


    //Remover for the syncronisation items
    struct shm_sync_remove
    {
        shm_sync_remove() { shared_memory_object::remove("SyncMemory"); }
        ~shm_sync_remove(){ shared_memory_object::remove("SyncMemory"); }
    } sync_remover;

    managed_shared_memory sync_segment  (create_only,
                                        "SyncMemory",
                                        1024);

    offset_ptr<sync_items> sync_data = sync_segment.construct<sync_items>("SyncItems")();

    int status = 0;
    pid_t pid;
    pid_t child_proc = fork();

    if (child_proc == 0) { // Child process
        std::cout << "child enter consuming func" << std::endl;
        take_points_from_buffer();

    } else if (child_proc > 0) { // Parent process
        std::cout << "parent enter producing func" << std::endl;
        add_points_to_buffer();

    } else { // Fork failed
        std::cout << "Fork() failed. Aborting" << std::endl;
        return 1;
    }

    // Destroy circular buffer  and sync items from memory
    segment.destroy<MyCircularBuffer>("MyCircularBuffer");
    sync_segment.destroy<sync_items>("SyncItems");

    return 0;
}

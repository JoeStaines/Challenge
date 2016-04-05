#define BOOST_ALL_NO_LIB

#include <boost/interprocess/containers/vector.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
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

void add_points_to_buffer() {
    // Open shared memory segment
    managed_shared_memory segment(open_only, "MySharedMemory");

    // Open circular buffer from shared memory
    offset_ptr<MyCircularBuffer> circ_buffer = segment.find<MyCircularBuffer>("MyCircularBuffer").first;

    // Fill buffer with random ints
    // (increase scale later)
    for (int i = 0; i < 1000; ++i) {
        circ_buffer->push_back(rand() % 100);
    }
}

int main (int argc, char* argv[])
{
    srand(time(NULL));
    if (argc == 1) {
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
        add_points_to_buffer();

        //Launch child process
        std::string s(argv[0]); s += " child ";
        if(0 != std::system(s.c_str()))
            return 1;

        //Check child has destroyed the vector
        if(segment.find<MyCircularBuffer>("MyCircularBuffer").first)
            return 1;
    } else { // child process
        //Open managed memory segment
        managed_shared_memory segment(open_only, "MySharedMemory");

        // Find circular buffer in shared memory
        offset_ptr<MyCircularBuffer> circ_buffer = segment.find<MyCircularBuffer>("MyCircularBuffer").first;

        boost::circular_buffer<int, ShmemAllocator>::iterator iter = circ_buffer->begin();
        for (iter; iter != circ_buffer->end(); ++iter) {
            std::cout << *iter << std::endl;
        }

        // Destroy circular buffer from memory
        segment.destroy<MyCircularBuffer>("MyCircularBuffer");
    }
   return 0;
}

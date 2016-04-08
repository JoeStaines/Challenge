#ifndef SHAREDMEMORYDATA_H_INCLUDED
#define SHAREDMEMORYDATA_H_INCLUDED

#include <boost/interprocess/containers/vector.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

namespace shmem_data {
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
}

#endif // SHAREDMEMORYDATA_H_INCLUDED

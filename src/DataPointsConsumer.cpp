#include "DataPointsConsumer.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "SharedMemoryData.h"


DataPointsConsumer::DataPointsConsumer(std::string pointsStr, std::string syncStr)
    : shmemPointsStr(pointsStr)
    , shmemSyncStr(syncStr)
{
    // Include later in arguments a HDF5 base class that can be extend with different save() virtual function
}

void DataPointsConsumer::take_points_from_buffer()
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer  and sync data from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>("SyncItems").first;

    while(!sync_data->finished) {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        if (circ_buffer->empty()) {
            //std::cout << "Process 2 buffer empty, waiting..." << std::endl;
            sync_data->cond_empty.wait(lock);
            //std::cout << "Process 2 waking up" << std::endl;

            // Use continue so that consumer skips taking out of the buffer
            // and checks the loop condition again + empty condition.
            continue;
        }

        //std::cout << "Process 2 taking data " << circ_buffer->front() << std::endl;
        circ_buffer->pop_front();
    }
}

DataPointsConsumer::~DataPointsConsumer()
{
    //dtor
}

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

int DataPointsConsumer::consume()
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer  and sync data from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>("SyncItems").first;

    {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        while (circ_buffer->empty()) {
            sync_data->cond_empty.wait(lock);
        }

        //std::cout << "Process 2 taking data " << circ_buffer->front() << std::endl;
        int val = circ_buffer->front();
        circ_buffer->pop_front();
        return val;
    }
}

std::vector<int> DataPointsConsumer::consume_to_vector(int limit, int poisonPill)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer  and sync data from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>("SyncItems").first;

    std::vector<int> data;

    while (data.size() < limit) {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        while (circ_buffer->empty()) {
            std::cout << "Consumer waiting on empty" << std::endl;
            sync_data->cond_empty.wait(lock);
            std::cout << "Consumer woke up" << std::endl;
        }

        data.push_back(circ_buffer->front());

        // Break out if we read the poison pill and return data vector early
        if (circ_buffer->front() == poisonPill)
            break;

        circ_buffer->pop_front();
    }

    return data;
}

DataPointsConsumer::~DataPointsConsumer()
{
    //dtor
}

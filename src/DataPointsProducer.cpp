#include "DataPointsProducer.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <chrono>
#include <thread>

#include "SharedMemoryData.h"

DataPointsProducer::DataPointsProducer(const std::string &pointsStr,
                                        const std::string &syncStr,
                                        const std::string &bufferStr,
                                        const std::string &syncItemsStr)
    : shmemPointsStr(pointsStr)
    , shmemSyncStr(syncStr)
    , shmemBufferStr(bufferStr)
    , shmemSyncItemsStr(syncItemsStr)
{
    std::cout << "producer constructor" << std::endl;
}

void DataPointsProducer::produce(int dataCount)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(shmemBufferStr.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(shmemSyncItemsStr.c_str()).first;

    auto start = std::chrono::system_clock::now();
    // Fill buffer with random ints
    // (increase scale later)
    {
    scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        for (int i = 0; i < dataCount; ++i) {
            int num = rand() % 10;
            circ_buffer->push_back(num);

        }
    sync_data->cond_empty.notify_one(); // Notify that circular buffer not empty
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time: " << diff.count() << std::endl;
}

void DataPointsProducer::finish(int poisonPill)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(shmemBufferStr.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(shmemSyncItemsStr.c_str()).first;

    // Poison pill chosen by the caller to determine when consumers should quit
    circ_buffer->push_back(poisonPill);
    sync_data->cond_empty.notify_one();
}

DataPointsProducer::~DataPointsProducer()
{
    //dtor
}

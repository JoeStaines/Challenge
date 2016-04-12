#include "DataPointsProducer.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <chrono>
#include <thread>


DataPointsProducer::DataPointsProducer(const shared_data_strings &dataStrings)
    : dataStrings(dataStrings)
{
    std::cout << "producer constructor" << std::endl;
}

void DataPointsProducer::produce(int dataCount)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, dataStrings.bufferMemory.c_str());
    managed_shared_memory sync_segment(open_only, dataStrings.syncMemory.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(dataStrings.bufferObject.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(dataStrings.syncObject.c_str()).first;

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
    managed_shared_memory segment(open_only, dataStrings.bufferMemory.c_str());
    managed_shared_memory sync_segment(open_only, dataStrings.syncMemory.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(dataStrings.bufferObject.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(dataStrings.syncObject.c_str()).first;

    {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        // Poison pill chosen by the caller to determine when consumers should quit
        circ_buffer->push_back(poisonPill);
        sync_data->cond_empty.notify_one();
    }
}

void DataPointsProducer::push(int val)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, dataStrings.bufferMemory.c_str());
    managed_shared_memory sync_segment(open_only, dataStrings.syncMemory.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(dataStrings.bufferObject.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(dataStrings.syncObject.c_str()).first;

    {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        // Poison pill chosen by the caller to determine when consumers should quit
        circ_buffer->push_back(val);
        sync_data->cond_empty.notify_one();
    }
}

void DataPointsProducer::push(const std::vector<int> &data)
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, dataStrings.bufferMemory.c_str());
    managed_shared_memory sync_segment(open_only, dataStrings.syncMemory.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>(dataStrings.bufferObject.c_str()).first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>(dataStrings.syncObject.c_str()).first;

    {
        scoped_lock<interprocess_mutex> lock (sync_data->mutex);
        for (auto it = data.begin(); it != data.end(); ++it) {
            circ_buffer->push_back(*it);
        }
        sync_data->cond_empty.notify_one();
    }
}

DataPointsProducer::~DataPointsProducer()
{
    //dtor
}

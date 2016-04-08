#include "DataPointsProducer.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <chrono>
#include <thread>

#include "SharedMemoryData.h"

DataPointsProducer::DataPointsProducer(std::string pointsStr, std::string syncStr, int dataCount)
    : shmemPointsStr(pointsStr)
    , shmemSyncStr(syncStr)
    , dataCount(dataCount)
{

}

void DataPointsProducer::add_points_to_buffer()
{
    using namespace boost::interprocess;
    // Open shared memory segment
    managed_shared_memory segment(open_only, shmemPointsStr.c_str());
    managed_shared_memory sync_segment(open_only, shmemSyncStr.c_str());

    // Open circular buffer from shared memory
    offset_ptr<shmem_data::MyCircularBuffer> circ_buffer = segment.find<shmem_data::MyCircularBuffer>("MyCircularBuffer").first;
    offset_ptr<shmem_data::sync_items> sync_data = sync_segment.find<shmem_data::sync_items>("SyncItems").first;
    for (int j = 0; j < 5; ++j) {

        auto start = std::chrono::system_clock::now();
        {
            scoped_lock<interprocess_mutex> lock (sync_data->mutex);
            // Fill buffer with random ints
            // (increase scale later)
            for (int i = 0; i < dataCount; ++i) {
                //std::this_thread::sleep_for (std::chrono::milliseconds(10)); // Just testing  if concurrent
                int num = rand() % 10;
                //std::cout << "Process 1 post data " << num << std::endl;
                circ_buffer->push_back(num);
            }
            sync_data->cond_empty.notify_one(); // Notify that circular buffer not empty

        }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Time: " << diff.count() << std::endl;

        std::cout << "Process 1 sleep..." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(1));

    }

    sync_data->finished = true;
    sync_data->cond_empty.notify_one(); // Use this to wake up consumer last time in case it's stuck waiting
}

DataPointsProducer::~DataPointsProducer()
{
    //dtor
}

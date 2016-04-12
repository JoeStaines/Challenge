#include <boost/interprocess/managed_shared_memory.hpp>

#include <iostream>
#include <vector>

#include "DataPointsConsumer.h"
#include "SharedMemoryData.h"
#include <gtest/gtest.h>

class ConsumerTest : public ::testing::Test
{

    public:
       DataPointsConsumer * consumer;

       virtual void SetUp()
       {
            shm_remove remover("MySharedMemory");
            shm_remove sync_remover("SyncMemory");

            shared_data_strings dataStrings = {"MySharedMemory", "SyncMemory", "MyCircularBuffer", "SyncItems"};

            consumer = new DataPointsConsumer(dataStrings);
       }

       virtual void TearDown()
       {
            delete consumer;
       }

};

TEST_F(ConsumerTest, ConsumeOneItems)
{
    using namespace boost::interprocess;
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory sync_segment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = segment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circBuffer->set_capacity(1);

    sync_segment.construct<shmem_data::sync_items>("SyncItems")();

    circBuffer->push_back(0);
    int val = consumer->consume();

    ASSERT_EQ(val, 0);
}

TEST_F(ConsumerTest, ConsumeTwoItems)
{
    using namespace boost::interprocess;
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory sync_segment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = segment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circBuffer->set_capacity(2);

    sync_segment.construct<shmem_data::sync_items>("SyncItems")();

    circBuffer->push_back(0);
    circBuffer->push_back(1);
    int val = consumer->consume();
    int val2 = consumer->consume();

    ASSERT_EQ(val, 0);
    ASSERT_EQ(val2, 1);
}

TEST_F(ConsumerTest, ConsumeOneItemToVector)
{
    using namespace boost::interprocess;
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory sync_segment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = segment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circBuffer->set_capacity(2);

    sync_segment.construct<shmem_data::sync_items>("SyncItems")();

    circBuffer->push_back(0);
    std::vector<int> vec = consumer->consume_to_vector(1, 100);

    ASSERT_EQ(vec.back(), 0);
}

TEST_F(ConsumerTest, ConsumeTwoItemsToVector)
{
    using namespace boost::interprocess;
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory sync_segment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = segment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circBuffer->set_capacity(2);

    sync_segment.construct<shmem_data::sync_items>("SyncItems")();

    circBuffer->push_back(0);
    circBuffer->push_back(1);
    std::vector<int> vec = consumer->consume_to_vector(2, 100);

    ASSERT_EQ(vec.back(), 1);
    vec.pop_back();
    ASSERT_EQ(vec.back(), 0);
}

TEST_F(ConsumerTest, ConsumeItemsToVectorPoisonPillBreak)
{
    using namespace boost::interprocess;
    managed_shared_memory segment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory sync_segment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator alloc_inst (segment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> circBuffer = segment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (alloc_inst);
    circBuffer->set_capacity(5);

    sync_segment.construct<shmem_data::sync_items>("SyncItems")();

    circBuffer->push_back(0);
    circBuffer->push_back(1);
    circBuffer->push_back(100);
    std::vector<int> vec = consumer->consume_to_vector(10, 100); // Limit is 10 but will break after 3

    ASSERT_EQ(vec.back(), 100);
    vec.pop_back();
    ASSERT_EQ(vec.back(), 1);
    vec.pop_back();
    ASSERT_EQ(vec.back(), 0);
}

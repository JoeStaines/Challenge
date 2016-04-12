#include <boost/interprocess/managed_shared_memory.hpp>

#include <iostream>
#include <vector>

#include "DataPointsProducer.h"
#include "SharedMemoryData.h"
#include <gtest/gtest.h>



class ProducerTest : public ::testing::Test
{
    public:
        DataPointsProducer *producer;

        virtual void SetUp()
        {
            shm_remove remover("MySharedMemory");
            shm_remove sync_remover("SyncMemory");

            shared_data_strings dataStrings = {"MySharedMemory", "SyncMemory", "MyCircularBuffer", "SyncItems"};

            producer = new DataPointsProducer(dataStrings);
        }

        virtual void TearDown()
        {
            delete producer;
        }
};


TEST_F(ProducerTest, ProduceZeroItems)
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

    producer->produce(0);

    ASSERT_EQ(circBuffer->size(), 0) << "Buffer size " << circBuffer->size() << " does not equal 0";
}

TEST_F(ProducerTest, ProduceOneItem)
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

    producer->produce(1);

    ASSERT_EQ(circBuffer->size(), 1) << "Buffer size " << circBuffer->size() << " does not equal 1";
}

TEST_F(ProducerTest, ProduceTwoItems)
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

    producer->produce(2);

    ASSERT_EQ(circBuffer->size(), 2) << "Buffer size " << circBuffer->size() << " does not equal 2";
}

/*** Test push functions ***/

TEST_F(ProducerTest, PushVal)
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

    producer->push(1);

    ASSERT_EQ(circBuffer->back(), 1);
}

TEST_F(ProducerTest, PushVector)
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

    std::vector<int> data;
    data.push_back(1);
    producer->push(data);

    ASSERT_EQ(circBuffer->back(), 1);
}

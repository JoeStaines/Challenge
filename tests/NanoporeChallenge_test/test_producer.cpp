#include <boost/interprocess/managed_shared_memory.hpp>

#include <iostream>

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

            producer = new DataPointsProducer("MySharedMemory", "SyncMemory", "MyCircularBuffer", "SyncItems");
        }

        virtual void TearDown()
        {
            delete producer;
        }
};


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

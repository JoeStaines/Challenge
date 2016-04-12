#include <boost/interprocess/managed_shared_memory.hpp>

#include <iostream>
#include <vector>

#include "DataPointsModifier.h"
#include "SharedMemoryData.h"
#include <gtest/gtest.h>

class ModifierTest : public ::testing::Test
{
    public:
        DataPointsModifier * modifier;

        virtual void SetUp()
        {
            shm_remove oldRemover("MySharedMemory");
            shm_remove oldSyncRemover("SyncMemory");

            shm_remove newRemover("NewBufferMemory");
            shm_remove newSyncRemover("NewSync");

            modifier = new DataPointsModifier();;
        }

        virtual void TearDown()
        {
            delete modifier;
        }
};

TEST_F(ModifierTest, ModifyValue)
{
    using namespace boost::interprocess;
    managed_shared_memory oldSegment(create_only,
                                 "MySharedMemory",  //segment name
                                 1024);

    managed_shared_memory oldSyncSegment(create_only,
                                 "SyncMemory",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator old_alloc_inst (oldSegment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> oldCircBuffer = oldSegment.construct<shmem_data::MyCircularBuffer>("MyCircularBuffer") (old_alloc_inst);
    oldCircBuffer->set_capacity(2);

    oldSyncSegment.construct<shmem_data::sync_items>("SyncItems")();

    // Create shared meory for new buffer with associated objects
    using namespace boost::interprocess;
    managed_shared_memory newSegment(create_only,
                                 "NewBufferMemory",  //segment name
                                 1024);

    managed_shared_memory newSyncSegment(create_only,
                                 "NewSync",  //segment name
                                 1024);

    //Initialize the STL-like allocator
    const shmem_data::ShmemAllocator new_alloc_inst (newSegment.get_segment_manager());

    offset_ptr<shmem_data::MyCircularBuffer> newCircBuffer = newSegment.construct<shmem_data::MyCircularBuffer>("NewCircBuffer") (new_alloc_inst);
    newCircBuffer->set_capacity(2);

    newSyncSegment.construct<shmem_data::sync_items>("NewSyncItems")();

    // Test modify function
    shared_data_strings readBufferStrings = {"MySharedMemory", "SyncMemory", "MyCircularBuffer", "SyncItems"};
    shared_data_strings writeBufferStrings = {"NewBufferMemory", "NewSync", "NewCircBuffer", "NewSyncItems"};
    modifier->set_read_buffer(readBufferStrings);
    modifier->set_write_buffer(writeBufferStrings);

    int poisonPill = -100;
    oldCircBuffer->push_back(1);
    oldCircBuffer->push_back(poisonPill);

    modifier->modify(poisonPill);

    ASSERT_EQ(newCircBuffer->back(), poisonPill);
    newCircBuffer->pop_back();
    ASSERT_EQ(newCircBuffer->back(), 11);
}

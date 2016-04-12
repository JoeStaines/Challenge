#include "DataPointsModifier.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <vector>

#include "DataPointsConsumer.h"
#include "DataPointsProducer.h"

#include "SharedMemoryData.h"

DataPointsModifier::DataPointsModifier()
{

}

void DataPointsModifier::set_read_buffer(const shared_data_strings &oldDataStrings)
{
    this->oldDataStrings = oldDataStrings;
}

void DataPointsModifier::set_write_buffer(const shared_data_strings &newDataStrings)
{
    this->newDataStrings = newDataStrings;
}

void DataPointsModifier::modify(int poisonPill)
{
    // Start retriving and modifying values
    bool finished = false;
    DataPointsConsumer consumer(oldDataStrings);
    DataPointsProducer producer(newDataStrings);

    while (!finished) {
        std::vector<int> data = consumer.consume_to_vector(100000, poisonPill);
        if (data.back() == poisonPill) {
            finished = true;
        }

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (*it != poisonPill)
                *it += 10;
        }

        producer.push(data);

    }

}

DataPointsModifier::~DataPointsModifier()
{
    //dtor
}

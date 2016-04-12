#ifndef DATAPOINTSCONSUMER_H
#define DATAPOINTSCONSUMER_H

#include <iostream>
#include <vector>
#include "SharedMemoryData.h"

class DataPointsConsumer
{
    public:
        DataPointsConsumer(const shared_data_strings &dataStrings);
        ~DataPointsConsumer();

        int consume();
        std::vector<int> consume_to_vector(int limit, int poisonPill);
    protected:
    private:
        shared_data_strings dataStrings;
};

#endif // DATAPOINTSCONSUMER_H

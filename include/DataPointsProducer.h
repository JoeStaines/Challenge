#ifndef DATAPOINTSPRODUCER_H
#define DATAPOINTSPRODUCER_H

#include <iostream>
#include <vector>

#include "SharedMemoryData.h"

class DataPointsProducer
{
    public:
        DataPointsProducer(const shared_data_strings &dataStrings);
        virtual ~DataPointsProducer();

        void produce(int dataCount);
        void finish(int poisonPill);
        void push(int val);
        void push(const std::vector<int> &data);
    protected:
    private:
        shared_data_strings dataStrings;
};

#endif // DATAPOINTSPRODUCER_H

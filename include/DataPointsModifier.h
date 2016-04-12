#ifndef DATAPOINTSMODIFIER_H
#define DATAPOINTSMODIFIER_H

#include "DataPointsProducer.h"

#include "SharedMemoryData.h"

class DataPointsModifier
{
    public:
        DataPointsModifier();
        ~DataPointsModifier();

        void set_read_buffer(const shared_data_strings &oldDataStrings);

        void set_write_buffer(const shared_data_strings &newDataStrings);

        void modify(int poisonPill);
    protected:
    private:
        shared_data_strings oldDataStrings;
        shared_data_strings newDataStrings;


};

#endif // DATAPOINTSMODIFIER_H

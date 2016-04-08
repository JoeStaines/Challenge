#ifndef DATAPOINTSCONSUMER_H
#define DATAPOINTSCONSUMER_H

#include <iostream>

class DataPointsConsumer
{
    public:
        DataPointsConsumer(std::string pointsStr, std::string syncStr);
        ~DataPointsConsumer();

        void take_points_from_buffer();
    protected:
    private:
        std::string shmemPointsStr;
        std::string shmemSyncStr;
};

#endif // DATAPOINTSCONSUMER_H

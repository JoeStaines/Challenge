#ifndef DATAPOINTSPRODUCER_H
#define DATAPOINTSPRODUCER_H

#include <iostream>

class DataPointsProducer
{
    public:
        DataPointsProducer(std::string pointsStr, std::string syncStr, int dataCount);
        ~DataPointsProducer();

        void add_points_to_buffer();
    protected:
    private:
        std::string shmemPointsStr;
        std::string shmemSyncStr;
        int dataCount;
};

#endif // DATAPOINTSPRODUCER_H

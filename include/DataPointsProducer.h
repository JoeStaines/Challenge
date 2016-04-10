#ifndef DATAPOINTSPRODUCER_H
#define DATAPOINTSPRODUCER_H

#include <iostream>

class DataPointsProducer
{
    public:
        DataPointsProducer(const std::string &pointsStr,
                            const std::string &syncStr,
                            const std::string &bufferStr,
                            const std::string &syncItemsStr);
        ~DataPointsProducer();

        void produce(int dataCount);
        void finish(int poisonPill);
    protected:
    private:
        const std::string shmemPointsStr;
        const std::string shmemSyncStr;
        const std::string shmemBufferStr;
        const std::string shmemSyncItemsStr;
};

#endif // DATAPOINTSPRODUCER_H

#ifndef DATAPOINTSCONSUMER_H
#define DATAPOINTSCONSUMER_H

#include <iostream>
#include <vector>

class DataPointsConsumer
{
    public:
        DataPointsConsumer(const std::string &pointsStr,
                            const std::string &syncStr,
                            const std::string &bufferStr,
                            const std::string &syncItemsStr);
        ~DataPointsConsumer();

        int consume();
        std::vector<int> consume_to_vector(int limit, int poisonPill);
    protected:
    private:
        std::string shmemPointsStr;
        std::string shmemSyncStr;
        std::string shmemBufferStr;
        std::string shmemSyncItemsStr;
};

#endif // DATAPOINTSCONSUMER_H

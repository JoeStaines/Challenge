#ifndef DATAPOINTSCONSUMER_H
#define DATAPOINTSCONSUMER_H

#include <iostream>
#include <vector>

class DataPointsConsumer
{
    public:
        DataPointsConsumer(std::string pointsStr, std::string syncStr);
        ~DataPointsConsumer();

        int consume();
        std::vector<int> consume_to_vector(int limit, int poisonPill);
    protected:
    private:
        std::string shmemPointsStr;
        std::string shmemSyncStr;
};

#endif // DATAPOINTSCONSUMER_H

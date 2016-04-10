#ifndef HDF5HANDLERBASE_H
#define HDF5HANDLERBASE_H

#include <iostream>
#include <string>

#include <H5Cpp.h>

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

#include <vector>

class HDF5HandlerBase
{
    public:
        HDF5HandlerBase(const std::string &fileName, const std::string &datasetName);
        virtual ~HDF5HandlerBase();

        int create_file();
        int extendable_file();
        virtual void save(const std::vector<int> &dataPoints);
    protected:
    private:

        const H5std_string	FILE_NAME; //("h5tutr_dset.h5");
        const H5std_string	DATASETNAME; //("dset");

        H5File file;
        DataSet dataset;
};

#endif // HDF5HANDLERBASE_H

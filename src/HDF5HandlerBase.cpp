#include "HDF5HandlerBase.h"

#ifndef H5_NO_NAMESPACE
     using namespace H5;
#ifndef H5_NO_STD
     using std::cout;
     using std::endl;
#endif  // H5_NO_STD
#endif



HDF5HandlerBase::HDF5HandlerBase(const std::string &fileName, const std::string &datasetName)
    : FILE_NAME(H5std_string(fileName))
    , DATASETNAME(H5std_string(datasetName))
{


    try
    {

        Exception::dontPrint();

        file = H5File(FILE_NAME, H5F_ACC_TRUNC);

        hsize_t dims[1] = {0};
        hsize_t maxdims[1] = {H5S_UNLIMITED};
        hsize_t chunk_dims[1] = {10000};

        DataSpace dataspace = DataSpace(1,dims,maxdims);

        DSetCreatPropList prop;
        prop.setChunk(1, chunk_dims);

        dataset = file.createDataSet( DATASETNAME,
	                         PredType::STD_I32BE, dataspace, prop);

        prop.close();
        dataspace.close();
    } catch (Exception &error) {
        // Throw FileIException, DataSetIException, DataSpaceIException
        throw;
    }

}

HDF5HandlerBase::~HDF5HandlerBase()
{
    file.close();
}

void HDF5HandlerBase::save(const std::vector<int> &dataPoints) {

    // Return if no data to add
    if (dataPoints.size() < 1)
        return;

    // dataset.write needs not const value of data
    int *data = const_cast<int*>(&dataPoints[0]);

    // Determine value of
    hsize_t dimsext[1];
    dimsext[0] = dataPoints.size();

    hsize_t size[1];
    hsize_t offset[1];


    try {
        DataSpace filespace = dataset.getSpace();
        int ndims = filespace.getSimpleExtentNdims();
        hsize_t dims[ndims];
        filespace.getSimpleExtentDims(dims);

        size[0] = dims[0] + dimsext[0];
        dataset.extend(size);

        offset[0] = dims[0];
        filespace = dataset.getSpace();
        filespace.selectHyperslab(H5S_SELECT_SET, dimsext, offset);

        DataSpace memspace = DataSpace(1, dimsext, NULL);

        dataset.write(data, PredType::NATIVE_INT, memspace, filespace);

        filespace.close();
        memspace.close();

    } catch (Exception &error) {
        throw;
    }


}

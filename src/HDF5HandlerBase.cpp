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
    std::cout << "Calling file close" << std::endl;
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

int HDF5HandlerBase::create_file()
{
    // Try block to detect exceptions raised by any of the calls inside it
    try
    {
        // Turn off the auto-printing when failure occurs so that we can
        // handle the errors appropriately
        Exception::dontPrint();

        // Create a new file using the default property lists.
        //H5File file(FILE_NAME, H5F_ACC_TRUNC);

        // Create the data space for the dataset.
        hsize_t dims[2];               // dataset dimensions
        dims[0] = 2;
        dims[1] = 4;
        DataSpace dataspace(2, dims);

        // Create the dataset.
        DataSet dataset = file.createDataSet(DATASETNAME, PredType::STD_I32BE, dataspace);

        }  // end of try block

        // catch failure caused by the H5File operations
    catch(FileIException error)
    {
        error.printError();
        return -1;
    }

    // catch failure caused by the DataSet operations
    catch(DataSetIException error)
    {
        error.printError();
        return -1;
    }

    // catch failure caused by the DataSpace operations
    catch(DataSpaceIException error)
    {
        error.printError();
        return -1;
    }

    return 0;  // successfully terminated
}

int HDF5HandlerBase::extendable_file() {

    hsize_t dims[2] = {3,3};	        // dataset dimensions at creation
     hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
     hsize_t chunk_dims[2] ={2, 5};
     int	   data[3][3] = { {1, 1, 1},    // data to write
	                          {1, 1, 1},
	                          {1, 1, 1} };

     // Variables used in extending and writing to the extended portion of dataset

     hsize_t size[2];
     hsize_t offset[2];
     hsize_t dimsext[2] = {7, 3};         // extend dimensions
     int     dataext[7][3] = { {2, 3, 4},
	                              {2, 3, 4},
	                              {2, 3, 4},
	                              {2, 3, 4},
	                              {2, 3, 4},
	                              {2, 3, 4},
	                              {2, 3, 5} };

    // Try block to detect exceptions raised by any of the calls inside it
    try
    {
	// Turn off the auto-printing when failure occurs so that we can
	// handle the errors appropriately
	Exception::dontPrint();

    // Create a new file using the default property lists.
	H5File file("hdf5_extend.h5", H5F_ACC_TRUNC);

	// Create the data space for the dataset.  Note the use of pointer
	// for the instance 'dataspace'.  It can be deleted and used again
	// later for another dataspace.  An HDF5 identifier can be closed
	// by the destructor or the method 'close()'.
	DataSpace *dataspace = new DataSpace (2, dims, maxdims);

	// Modify dataset creation property to enable chunking
	DSetCreatPropList prop;
	prop.setChunk(2, chunk_dims);

	// Create the chunked dataset.  Note the use of pointer.
	DataSet *dataset = new DataSet(file.createDataSet( "dset",
	                         PredType::STD_I32BE, *dataspace, prop) );

	// Write data to dataset.
	dataset->write(data, PredType::NATIVE_INT);

	// Extend the dataset. Dataset becomes 10 x 3.
	size[0] = dims[0] + dimsext[0];
	size[1] = dims[1];
	dataset->extend(size);

	// Select a hyperslab in extended portion of the dataset.
	DataSpace *filespace = new DataSpace(dataset->getSpace ());
	offset[0] = 3;
	offset[1] = 0;
	filespace->selectHyperslab(H5S_SELECT_SET, dimsext, offset);

	// Define memory space.
	DataSpace *memspace = new DataSpace(2, dimsext, NULL);

	// Write data to the extended portion of the dataset.
	dataset->write(dataext, PredType::NATIVE_INT, *memspace, *filespace);

	// Close all objects and file.
	prop.close();
	delete filespace;
	delete memspace;
	delete dataspace;
	delete dataset;

	// ---------------------------------------
	// Re-open the file and read the data back
	// ---------------------------------------

	int        rdata[10][3];
	int        i,j, rank, rank_chunk;
	hsize_t    chunk_dimsr[2], dimsr[2];

	// Open the file and dataset.
	//file.openFile(FILE_NAME, H5F_ACC_RDONLY);
	dataset = new DataSet(file.openDataSet( DATASETNAME));

	// Get the dataset's dataspace and creation property list.
	filespace = new DataSpace(dataset->getSpace());
	prop = dataset->getCreatePlist();

	// Get information to obtain memory dataspace.
	rank = filespace->getSimpleExtentNdims();
	herr_t status_n = filespace->getSimpleExtentDims(dimsr);

	if (H5D_CHUNKED == prop.getLayout())
	     rank_chunk = prop.getChunk(rank, chunk_dimsr);
	cout << "rank chunk = " << rank_chunk << endl;;

	memspace = new DataSpace(rank, dimsr, NULL);
	dataset->read(rdata, PredType::NATIVE_INT, *memspace, *filespace);

	cout << endl;
	for (j = 0; j < dimsr[0]; j++) {
	    for (i = 0; i < dimsr[1]; i++)
	       cout << " " <<  rdata[j][i];
	    cout << endl;
	}

	// Close all objects and file.
	prop.close();
	delete filespace;
	delete memspace;
	delete dataset;
	file.close();

    }  // end of try block

    // catch failure caused by the H5File operations
    catch(FileIException error)
    {
	error.printError();
	return -1;
    }

    // catch failure caused by the DataSet operations
    catch(DataSetIException error)
    {
	error.printError();
	return -1;
    }

    // catch failure caused by the DataSpace operations
    catch(DataSpaceIException error)
    {
	error.printError();
	return -1;
    }

    return 0;  // successfully terminated
}

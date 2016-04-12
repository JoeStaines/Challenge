Build
=====

Built using Code::Blocks 13.12

Dependencies:
Boost 1.60 : http://www.boost.org/users/history/version_1_60_0.html
Googletest : https://github.com/google/googletest
HDF5 : https://www.hdfgroup.org/HDF5/

Compiler directories:
/include
(boost 1.6 root)
(hdf5 root)/include

Compiler flags:
-std=c++11

Linker directories:
(hdf5 root)/lib

Linker flags:
-lrt
-lhdf5_cpp
-lhdf5
-pthread

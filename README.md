Comments on challenge
=====================

Unfortunately not all the process are able to run parallel; you have to specify a command line argument either 1 or 2 to specify two different modes. '1' handles single producer/consumer and equates to processes 1 and 2 on the challenge (these are in parallel), while mode '2' equates to processes 1,2 and 4, where it modifies a current buffer and writes to a new one, but doesn't have another process reading the old buffer at the same time.

This is because of the way the buffer is handled. I use boost::circular_buffer as the container since it works with boost::interprocess but it isn't really suitable for multi consumer reading without actually consuming the input. The problem is the possibility of the producer overunning the consumer, i.e buffer of size 5, producer adds 4, consumer reads 2, then producer adds another 4 then consumer reads 1 and thinks it has caught up. 

I was thinking that the solution would have every consumer 'register' the index of where they are in the buffer and if the buffer starts adding a value that overwrites where a consumer is, the buffer can 'push' the index of that consumer so that it reads the next oldest value instead. Or make it block if you dont want any data lost at all. I didn't implement this as i could only implement simple custom structs into shared memory more complex containers have to start defining allocators for custom containers to make it work with shared memory and with not much documentation or examples on how to do that plus the time available, i stuck with the safe option.

Another problem was with the scope of defining a segment of shared memory. If the variable that you initialise to make the shared segment falls out of scope, it closes that portion of shared memory with it, which is nice for cleanup but means you have to keep the variable in scope at all times which makes designating it to its own function is a lot harder.

I also tried to move the code that is used to open some segment of shared memory into the constructor and have the segment be a class member variable, but when i tried to do this, it would result in segmentation faults. Technically the variable is still in scope while the object is still alive so i couldn't understand why it was seg faulting, but it means that all of the code to open shared memory had to be done where it was used, creating a lot of bloat. Trying to find a way to properly abstract away the opening of shared memory would probably be the first thing i would do to try and improve this program.

Overall it was a fun and - at times difficult - challenge, but a learned a lot at the same time.

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

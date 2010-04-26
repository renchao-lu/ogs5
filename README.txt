## OGS-5 ##
see https://geosys.ufz.de/trac for build instructions

## For those who are interested in ogs-pardiso on linux, please extract the libraries in LIB/MKL 

sh extract_mkl.sh

Then, compile with the option

mkdir Build
cd Build
cmake .. -D=OGS_FEM_MKL=on

NOTE: Since the executable, ogs, is created by linking the libraries dynamically, users still need to download MKL from the intel site and install them to properly run ogs. Otherwise, the executalbe complains missing libraries.

 

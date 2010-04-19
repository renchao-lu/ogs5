## OGS-5 ##
see https://geosys.ufz.de/trac for build instructions

## For those who are interested in ogs-pardiso on linux, please extract the libraries in LIB/MKL 

sh extract_mkl.sh

Then, compile with the option

make Build
cd Build
cmake .. -D=CONFIG_G++_PARDISO=on

 

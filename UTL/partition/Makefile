# Myles English 2008-10-20
# TODO: add more from UTL/makefile, should compile with the root make options
# linux-g++ and linux-mpi(?)

CXX = g++ # TODO: get the right option
CCFLAGS = -g -Wall # TODO: here too

MSHINCLUDE     = ../MSH
#MPIINCLUDE     = ../MPI
#ROOTINCLUDE    = ../

CXX_INCS      += -I$(MSHINCLUDE) #-I$(ROOTINCLUDE)

#ifdef MPI
#CXX_INCS      += -I$(MPIINCLUDE)
#endif

partition: partition.o itoa.o matrix_class.o mesh.o partition.o stdafx.o 
	${CXX} ${CCFLAGS} itoa.o matrix_class.o mesh.o stdafx.o partition.o -o partition

itoa.o: itoa.cpp itoa.h
	${CXX} ${CCFLAGS} -c itoa.cpp -o itoa.o

matrix_class.o: matrix_class.cpp matrix_class.h stdafx.h 
	${CXX} ${CCFLAGS} $(CXX_INCS) -c matrix_class.cpp -o matrix_class.o
	
mesh.o: mesh.cpp mesh.h matrix_class.h
	${CXX} ${CCFLAGS} -c mesh.cpp -o mesh.o
	
partition.o: partition.cpp matrix_class.h mesh.h
	${CXX} ${CCFLAGS} -c partition.cpp -o partition.o
	
stdafx.o: stdafx.cpp stdafx.h
	${CXX} ${CCFLAGS} -c stdafx.cpp -o stdafx.o

distclean clean:
	rm -rf *.o
	rm -rf *~


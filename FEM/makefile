# GeoSys - Makefile for LINUX
#  04.11.2004 WW
#  17.12.2008 WW
ifneq ($(strip $(CHOSEN_TARGET)),)
include $(TOPDIR)/configs/$(CHOSEN_TARGET)
endif

## 17.12.2008 WW
ifdef NEW_SP
include objects_sp.mak
else
include objects.mak
endif
#######

FEMINCLUDE     = ../FEM
GEMINCLUDE     = ../GEM
GEOINCLUDE     = ../GEO
MSHINCLUDE     = ../MSH
UTLINCLUDE     = ../UTL
MPIINCLUDE     = ../MPI
ROOTINCLUDE    = ../

CXX_INCS      += -I$(FEMINCLUDE) -I$(GEMINCLUDE) -I$(GEOINCLUDE) -I$(MSHINCLUDE) -I$(UTLINCLUDE) -I$(ROOTINCLUDE)
ifdef MPI
CXX_INCS      += -I$(MPIINCLUDE)
endif

#### Only temporarliy until random walk compiles on every platform
## Commented by WW
#ifdef NO_RANDOM_WALK
#CXX_FLAGS       += -URANDOM_WALK
#else
#CXX_FLAGS       += -DRANDOM_WALK
#endif


.SUFFIXES: .o .cpp .h 
%.o:	%.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -c $<


.PHONY : $(CONFIG_NAMES)
$(CONFIG_NAMES): $(FEMOBJS)


distclean clean:
	rm -rf *.o
	rm -rf *~
	rm -rf *.ti


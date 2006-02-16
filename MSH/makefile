# GeoSys - Makefile for LINUX
# Last modification 04/11/2004 WW

ifneq ($(strip $(CHOSEN_TARGET)),)
include $(TOPDIR)/configs/$(CHOSEN_TARGET)
endif


#### Only temporarliy until random walk compiles on every platform
## Commented by WW. This option is already defined in source file
#ifdef NO_RANDOM_WALK
#CXX_FLAGS       += -URANDOM_WALK
#else
#CXX_FLAGS       += -DRANDOM_WALK
#endif


include objects.mak

FEMINCLUDE     = ../FEM
GEOINCLUDE     = ../GEO
MSHINCLUDE     = ../MSH
ROOTINCLUDE    = ../

CXX_INCS      += -I$(FEMINCLUDE) -I$(GEOINCLUDE) -I$(MSHINCLUDE) -I$(ROOTINCLUDE)


.SUFFIXES: .o .cpp .h 
%.o:	%.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -c $<


.PHONY : $(CONFIG_NAMES)
$(CONFIG_NAMES): $(MSHOBJS)


distclean clean:
	rm -rf *.o
	rm -rf *~
	rm -rf *.ti

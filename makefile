#GeoSys makfile file
# 04/11/2004 WW
TARGET = rf4
include link.mak


all:
	make -C FEM -f makefile.gcc
	make -C GEO -f makefile.gcc
	make -C MSH -f makefile.gcc
	g++ -o $(TARGET) $(FEMOBJS)  $(GEOOBJS)  $(MSHOBJS) -lm

linux:
	make -C FEM -f makefile.gcc
	make -C GEO -f makefile.gcc
	make -C MSH -f makefile.gcc


	g++ -o $(TARGET) $(FEMOBJS)  $(GEOOBJS)  $(MSHOBJS)  -lm

NEC:
	make -C FEM -f makefile.sxc
	make -C GEO -f makefile.sxc
	make -C MSH -f makefile.sxc
	sxc++ -o $(TARGET) $(FEMOBJS)  $(GEOOBJS)  $(MSHOBJS) -lm



clean:   
	make -C FEM -f makefile.gcc clean
	make -C GEO -f makefile.gcc clean
	make -C MSH -f makefile.gcc clean
	rm -rf  $(TARGET)
	rm -rf  gs_project.o 


           


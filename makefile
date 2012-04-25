CC = gcc
LIB = -L($LD_LIBRARY_PATH)
INC = -I../../ati-stream-sdk-v2.3-lnx32/include
PROGPATH = ConwayComputeCUI

ConwayCompute : ConwayComputeCUI.o dispatch.o fileio.o output.o platform_lnx.o platform_osx.o platform_win.o setup.o swap.o makefile
	$(CC) -o ConwayCompute ConwayComputeCUI.o dispatch.o fileio.o output.o platform_lnx.o platform_osx.o platform_win.o setup.o swap.o ($LD_LIBRARY_PATH)/libglut.so ($LD_LIBRARY_PATH)/libOpenCL.so

ConwayComputeCUI.o : $(PROGPATH)/ConwayComputeCUI.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/ConwayComputeCUI.cpp

dispatch.o : $(PROGPATH)/dispatch.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/dispatch.cpp

fileio.o : $(PROGPATH)/fileio.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/fileio.cpp

output.o : $(PROGPATH)/output.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/output.cpp

platform_lnx.o : $(PROGPATH)/platform_lnx.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/platform_lnx.cpp

platform_osx.o : $(PROGPATH)/platform_osx.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/platform_osx.cpp

platform_win.o : $(PROGPATH)/platform_win.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/platform_win.cpp

setup.o : $(PROGPATH)/setup.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/setup.cpp

swap.o : $(PROGPATH)/swap.cpp makefile
	$(CC) $(LIB) $(INC) -c $(PROGPATH)/swap.cpp
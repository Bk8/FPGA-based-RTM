# This is the Makfile for Maxeler compiler FPGA. It is modified from
# the default Makfile from the tutorial, which is only support for single C
# file. This Makfile is able to support multiple C/C++ source files.
# However, it may be not robust enough without full test. Any question
# could be consulted to heconghui@gmail.com
#
# The following variable should be filled in
# 1. BASEDIR			
# 2. PACKAGE			
# 3. APP					
# 4. HOSTCODE			
# 5. C_SOURCES		
# 6. CPP_SOURCES 	
# 7. INCLUDE_PATH	
# 8. LDFLAGS			
#
#
# Include the master makefile.include
nullstring :=
space := $(nullstring) # comment
MAXCOMPILERDIR_QUOTE:=$(subst $(space),\ ,$(MAXCOMPILERDIR))

compile : run-sim
ifndef MAXCOMPILERDIR
	$(error MAXCOMPILERDIR environment variable is not set)
endif
nullstring :=
space := $(nullstring) # a space at the end
MAXCOMPILERDIR_QUOTE:=$(subst $(space),\ ,$(MAXCOMPILERDIR))
include $(MAXCOMPILERDIR_QUOTE)/lib/Makefile.include

# Root of the project directory tree
BASEDIR=..
# Java package name
PACKAGE=
# Application name
APP=PassThrough

# Names of your maxfiles
HWMAXFILE=$(APP).max
HOSTSIMMAXFILE=$(APP)HostSim.max

# Java application builders
HWBUILDER=$(APP)HWBuilder.java
HOSTSIMBUILDER=$(APP)HostSimBuilder.java
SIMRUNNER=$(APP)SimRunner.java

# C/CPP host code
HOSTCODE=$(APP)HostCode.cpp

# Aux C Code
HEADERS           := $(wildcard *.h)

# c sources, separated by space
C_SOURCES 				:=
C_OBJS          	:= $(patsubst %.c, %.o, $(C_SOURCES))

# cpp sources, separated by space
CPP_SOURCES       := auxiliary.cpp global_var.cpp 
CPP_OBJS          := $(patsubst %.cpp, %.o, $(CPP_SOURCES))

CC								?= gcc
CXX								?= g++

# include path, remember to add -I
INCLUDE_PATH += 
LDFLAGS+=$(MAXCOMPILER_LIBS)
CFLAGS+=$(MAXCOMPILER_INC) $(INCLUDE_PATH) -std=gnu99 -g
CXXFLAGS+=$(MAXCOMPILER_INC) $(INCLUDE_PATH) 

# Build in the current directory if MAXCOMPILER_BUILD_DIR is not set
ifndef MAXCOMPILER_BUILD_DIR
	export MAXCOMPILER_BUILD_DIR = $(BASEDIR)/hardware
endif


# Find the likely location of the .max file using maxGuessBuildDir
BUILD_RESULTS=`$(MAXGUESSBUILDDIR) -s $(basename $(HWMAXFILE))`results
BUILD_RESULTS_SIM=`$(MAXGUESSBUILDDIR) -s $(basename $(HOSTSIMMAXFILE))`results

BOARD_MODEL:=`java -cp $(BASEDIR)/bin:$(MAXCOMPILERDIR_QUOTE)/lib/MaxCompiler.jar config.BoardModel`

.PHONY: all clean app app-sim run run-sim javasim javacompile build-sim build-hw $(APP)SimFPGA.o $(APP)FPGA.o

all:
	@echo " "
	@echo " *******************************************************************************"
	@echo " Application Name = $(APP)"
	@echo " *******************************************************************************"
	@echo " "
	@echo " This makefile can build and run your accelerated applicaton"
	@echo " in either simulation or hardware."
	@echo " "
	@echo " Java targets:"
	@echo " "
	@echo "     make java-compile - Compile all of the Java source"
	@echo "     make java-sim     - Run Java simulation"
	@echo " 	                   ($(SIMRUNNER))"
	@echo "     make build-hw     - Build a hardware .max file"
	@echo " 	                   ($(HWBUILDER))"
	@echo "     make build-sim    - Build a simulation .max file"
	@echo " 	                   ($(HOSTSIMBUILDER))"
	@echo " "
	@echo " Host code targets ($(HOSTCODE).c):"
	@echo " "
	@echo "     make app-hw  - Build with a hardware .max file"
	@echo "     make run-hw  - Build and run with a hardware .max file"
	@echo "     make app-sim - Build with a simulation .max file"
	@echo "     make run-sim - Build and run with a simulation .max file"
	@echo " "
	@echo " Clean targets:"
	@echo " "
	@echo "     make clean          - Clean software object files, executables and"
	@echo "                           application output"
	@echo "     make clean-maxfiles - Cleans hardware and simulation build results,"
	@echo "                           including hardware .max files"
	@echo "                           MAKE SURE YOU REALLY WANT TO DO THIS!"
	@echo " "
	@echo " Your current environment:"
	@echo "     MAXCOMPILERDIR        = $(MAXCOMPILERDIR)"
	@echo "     MAXELEROSDIR          = $(MAXELEROSDIR)"
	@echo "     MAXCOMPILER_BUILD_DIR = $(MAXCOMPILER_BUILD_DIR)"
	@echo " "

################## Java build ################

OLD_DIR=$(shell pwd)
java-compile:
	cd $(BASEDIR) ; ant -f $(OLD_DIR)/build.xml -Dbasedir=.

java-sim: java-compile
# APP name must be set
ifeq ($(APP),)
	$(error variable APP not set)
endif
ifneq ($(PACKAGE),)
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(PACKAGE)/$(basename $(SIMRUNNER))
else
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(basename $(SIMRUNNER))
endif

build-sim: java-compile
# APP name must be set
ifeq ($(APP),)
	$(error variable APP not set)
endif
ifneq ($(PACKAGE),)
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(PACKAGE)/$(basename $(HOSTSIMBUILDER))
else
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(basename $(HOSTSIMBUILDER))
endif

build-hw: java-compile
# APP name must be set
ifeq ($(APP),)
	$(error variable APP not set)
endif
ifneq ($(PACKAGE),)
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(PACKAGE)/$(basename $(HWBUILDER))
else
	MAXSOURCEDIRS=$(BASEDIR)/src MAXAPPJCP=$(BASEDIR)/bin $(MAXJAVARUN) $(basename $(HWBUILDER))
endif

clean-maxfiles:
	-rm -rf $(BASEDIR)/bin
	-rm -rf $(BUILD_RESULTS_SIM)
	-rm -rf $(BUILD_RESULTS)

################## Host Code for hardware .max files ################

$(APP)FPGA.o:
	$(MAXFILECOMPILE) "$(BUILD_RESULTS)/$(HWMAXFILE)" "$@" "$(APP)"

$(basename $(HOSTCODE)).o: $(HOSTCODE)
ifeq ($(shell echo $(HOSTCODE) | sed 's/.*\.//g'), c)
	$(CC) $(HOSTCODE) $(CFLAGS)  -DSIM -DDESIGN_NAME=$(APP) -c -o $(basename $(HOSTCODE))Sim.o
else
	$(CXX) $(HOSTCODE) $(CXXFLAGS)  -DSIM -DDESIGN_NAME=$(APP) -c -o $(basename $(HOSTCODE))Sim.o
endif

$(APP)Run: $(basename $(HOSTCODE)).o $(APP)FPGA.o $(OTHER_OBJECTS) $(CPP_OBJS) $(C_OBJS)
	$(CXX) -o $(APP)Run $^ $(LDFLAGS)

app-hw: $(APP)Run

run-hw: $(APP)Run
	./$(APP)Run

################## Host Code for simulation .max files ################

$(APP)SimFPGA.o:
	$(MAXFILECOMPILE) "$(BUILD_RESULTS_SIM)/$(HOSTSIMMAXFILE)" "$@" "$(APP)"

# auto select between C and CPP code
$(basename $(HOSTCODE))Sim.o: $(HOSTCODE) $(CPP_OBJS) $(C_OBJS)
ifeq ($(shell echo $(HOSTCODE) | sed 's/.*\.//g'), c)
	$(CC) $(HOSTCODE) $(CFLAGS)  -DSIM -DDESIGN_NAME=$(APP) -c -o $(basename $(HOSTCODE))Sim.o 
else
	$(CXX) $(HOSTCODE) $(CXXFLAGS)  -DSIM -DDESIGN_NAME=$(APP) -c -o $(basename $(HOSTCODE))Sim.o
endif

$(APP)SimRun: $(APP)SimFPGA.o  $(OTHER_OBJECTS) $(C_OBJS) $(CPP_OBJS) $(basename $(HOSTCODE))Sim.o
	$(CXX) -o $(APP)SimRun  $^ $(LDFLAGS)

app-sim: $(APP)SimRun

run-sim: $(APP)SimRun
	$(MAXCOMPILERSIM) -c$(BOARD_MODEL) restart
	LD_LIBRARY_PATH="$(MAXCOMPILERDIR)/lib/maxeleros-sim/lib/:$(LD_LIBRARY_PATH)" LD_PRELOAD="libmaxeleros.so" ./$(APP)SimRun "sim0:sim"
	$(MAXCOMPILERSIM) stop

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.c %.h
	$(CC) -c $(CFLAGS)  -o $@ $<

################## Clean ################

clean::
	-$(MAXCOMPILERSIM) stop
	-rm -rf *.o $(CPP_OBJS) $(C_OBJS) $(APP)SimRun $(APP)Run watch_*.bin watch_*.csv core.*

clean-all :
	make clean
	make clean-maxfiles

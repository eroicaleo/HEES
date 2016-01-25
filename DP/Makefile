
BOOST_INC_DIR    = "$(HOME)/usr/include"
BOOST_LIB_DIR    = "$(HOME)/usr/lib/boost"

CFLAG        = -c -g -Wall -I$(BOOST_INC_DIR)
CFLAG_DBG    = $(CFLAG) -DDEBUG_VERBOSE
CFLAG_DP     = $(CFLAG_DBG) -DDP_BINARY 
CFLAG_SCHED  = $(CFLAG_DBG) -DSWAP_MAIN

EXT_OBJ   = ../nnet/nnetmultitask.o ../nnet/nnet.o ../nnet/util.o \
            ../DCCon_in.o ../VoltageTable.o ../powersource.o \
            ../ParseCommandLine.o ../ScheduleBuilder.o ../TaskGenerator.o \
			../selModel/testFilesystem.o ../TaskHandoff.o

BOOST_LIBS = -lboost_program_options -lboost_system -lboost_filesystem

binary = dp single sched solar

dp = taskScheduling

dp : $(dp)_bin.o $(EXT_OBJ)
	g++ $^ -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

all : $(EXT_OBJ) taskScheduling.o SwapScheduling.o testSchedDVFS.o
	g++ $^ -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

dvfs : $(EXT_OBJ) taskScheduling.o SwapScheduling.o testDVFSOnly.o
	g++ $^ -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

single : $(dp).o $(EXT_OBJ) testSingleVoltage.o
	g++ $^ $(BOOST_LIBS) -o $@

sched : $(dp).o $(EXT_OBJ) testSched.o
	g++ $^ -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

solar : $(EXT_OBJ) $(dp)_dbg.o testVariableSolarPower.o
	g++ $^ -DDEBUG_VERBOSE -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

swap  : $(EXT_OBJ) taskScheduling.o SwapScheduling_bin.o
	g++ $^ -L$(HOME)/usr/lib/boost $(BOOST_LIBS) -o $@

SwapScheduling_bin.o : SwapScheduling.cpp
	g++ $(CFLAG_SCHED) $< -o $@

SwapScheduling.o : SwapScheduling.cpp
	g++ $(CFLAG_DBG) $< -o $@

taskScheduling.o : taskScheduling.cpp
	g++ $(CFLAG_DBG) $< -o $@

%_bin.o : %.cpp
	g++ $(CFLAG_DP) $< -o $@

%_dbg.o : *.cpp
	g++ $(CFLAG_DBG) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

%.o : %.c
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o $(binary)

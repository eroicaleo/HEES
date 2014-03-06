
CFLAG = -c -g -Wall -DDEBUG_VERBOSE
CFLAG_DP = $(CFLAG) -DDP_BINARY -DDEBUG_VERBOSE
EXT_OBJ = ../nnet/nnetmultitask.o ../nnet/nnet.o ../nnet/util.o \
          ../DCCon_in.o ../VoltageTable.o ../powersource.o ../ParseCommandLine.o ../ScheduleBuilder.o
BOOST_LIBS = -lboost_program_options

dp = taskScheduling

dp : $(dp)_bin.o $(EXT_OBJ)
	g++ $^ $(BOOST_LIBS) -o $@

all : $(dp).o

single : $(dp).o $(EXT_OBJ) testSingleVoltage.o
	g++ $^ $(BOOST_LIBS) -o $@

%_bin.o : %.cpp
	g++ $(CFLAG_DP) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

%.o : %.c
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o dp

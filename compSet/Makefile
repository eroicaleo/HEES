
CFLAG = -c -g -Wall
CFLAG_CATS = $(CFLAG) -DCATS_BINARY
EXT_OBJ = ../nnet/nnetmultitask.o ../nnet/nnet.o ../nnet/util.o \
	../VoltageTable.o ../ScheduleBuilder.o  ../DP/taskScheduling.o ../ParseCommandLine.o ../DCCon_in.o ../powersource.o
BOOST_LIBS = -lboost_program_options

ca_ts_fixed: ca_ts_fixed_bin.o
	g++ ca_ts_fixed_bin.o $(EXT_OBJ) $(BOOST_LIBS) -o ca_ts_fixed

ca_ts: ca_ts_bin.o
	g++ ca_ts_bin.o -o ca_ts

all : ca_ts.o ca_ts_fixed.o

%_bin.o : %.cpp
	g++ $(CFLAG_CATS) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o ca_ts ca_ts_fixed

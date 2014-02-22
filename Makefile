.PHONY : all

bin = energysys

OBJ = DCCon_out.o SuperCap.o main.o DischargeProcess.o ChargeProcess.o DCCon_in.o DCCon_dis.o LionBat.o LoadApp.o selVCTI.o \
			powersource.o DCSolver.o ees_bank.o HEESTimer.o ParseCommandLine.o

SCHED_OBJ = DCCon_in.o Scheduler.o VoltageTable.o

NNET_OBJ = nnet/nnet.o nnet/util.o nnet/nnetmultitask.o
DP_OBJ = DP/taskScheduling.o
CATS_OBJ = compSet/ca_ts.o compSet/ca_ts_fixed.o

CC = g++
# CFLAGS = -c -Wall -I/home/yzhang/usr/sundials/include
CFLAGS = -c -Wall -g -I/home/yzhang/usr/sundials/include -I/home/yzhang/usr/include/
LDFLAGS = --static -L/home/yzhang/usr/sundials/lib -L/home/yzhang/usr/lib/boost
# LDFLAGS = 
SUNDIALS_LIBS = -lsundials_kinsol -lsundials_nvecserial
BOOST_LIBS = -lboost_program_options
# SUNDIALS_LIBS = 

# CFLAGS += -DDEBUG_YANGGE
# CFLAGS += -DUSING_MATLAB

all : $(bin)

$(bin) : $(OBJ) nnet_obj dp_obj
	g++ $(LDFLAGS) $(OBJ) -o $@ $(SUNDIALS_LIBS) $(BOOST_LIBS) $(NNET_OBJ) $(DP_OBJ)

sched :  $(SCHED_OBJ) nnet_obj dp_obj cats_obj
	g++ $(NNET_OBJ) $(DP_OBJ) $(CATS_OBJ) $(SCHED_OBJ) -o Scheduler

%.o : %.cpp
	g++ $(CFLAGS) $< -o $@

nnet_obj :
	make -C nnet/ all

dp_obj :
	make -C DP/ all

cats_obj :
	make -C compSet/ all

clean :
	make -C nnet/ $@
	make -C DP/ $@
	make -C compSet/ $@
	rm -rf *.o $(bin) Scheduler

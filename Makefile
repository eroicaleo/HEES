.PHONY : all

##----------------------------------------------------------------------------------------------------
## You need to change this part based on the installation of your sundials library and boost library
##----------------------------------------------------------------------------------------------------
SUNDIALS_INC_DIR = "$(HOME)/usr/sundials/include"
SUNDIALS_LIB_DIR = "$(HOME)/usr/sundials/lib"

##----------------------------------------------------------------------------------------------------
## If boost library is installed locally
##----------------------------------------------------------------------------------------------------
BOOST_INC_DIR    = "$(HOME)/usr/include"
BOOST_LIB_DIR    = "$(HOME)/usr/lib/boost"

##----------------------------------------------------------------------------------------------------
## Maybe boost library is installed systemwise
## BOOST_INC_DIR    = "/usr/include"
## BOOST_LIB_DIR    = "/usr/lib/x86_64-linux-gnu"
##----------------------------------------------------------------------------------------------------

##----------------------------------------------------------------------------------------------------
## You will not need to change the following part
##----------------------------------------------------------------------------------------------------

bin       = energysys

OBJ       = DCCon_out.o SuperCap.o main.o DischargeProcess.o ChargeProcess.o DCCon_in.o DCCon_dis.o LionBat.o LoadApp.o selVCTI.o \
			powersource.o DCSolver.o ees_bank.o HEESTimer.o ParseCommandLine.o VoltageTable.o

SCHED_OBJ = DCCon_in.o Scheduler.o VoltageTable.o ScheduleBuilder.o ParseCommandLine.o powersource.o
NNET_OBJ  = nnet/nnet.o nnet/util.o nnet/nnetmultitask.o
DP_OBJ    = DP/taskScheduling.o
CATS_OBJ  = compSet/ca_ts.o compSet/ca_ts_fixed.o

CC            = g++
CFLAGS        = -c -Wall -g -I$(SUNDIALS_INC_DIR) -I$(BOOST_INC_DIR) -I/usr/include
LDFLAGS       = --static -L$(SUNDIALS_LIB_DIR) -L$(BOOST_LIB_DIR)
SUNDIALS_LIBS = -lsundials_kinsol -lsundials_nvecserial
BOOST_LIBS    = -lboost_program_options

all : $(bin)

$(bin) : $(OBJ)
	g++ $(LDFLAGS) $(OBJ) -o $@ $(SUNDIALS_LIBS) $(BOOST_LIBS)

sched :  $(SCHED_OBJ) nnet_obj dp_obj cats_obj
	g++ $(LDFLAGS) $(NNET_OBJ) $(DP_OBJ) $(CATS_OBJ) $(SCHED_OBJ) $(BOOST_LIBS) -o Scheduler

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

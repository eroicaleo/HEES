.PHONY : all

bin = energysys

OBJ = DCCon_out.o SuperCap.o main.o DischargeProcess.o ChargeProcess.o DCCon_in.o DCCon_dis.o LionBat.o LoadApp.o selVCTI.o \
			powersource.o DCSolver.o ees_bank.o HEESTimer.o ParseCommandLine.o

NNET_OBJ = nnet/nnet.o nnet/util.o
DP_OBJ = DP/taskScheduling.o

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

%.o : %.cpp
	g++ $(CFLAGS) $< -o $@

nnet_obj :
	make -C nnet/ all

dp_obj :
	make -C DP/ all

clean :
	make -C nnet/ $@
	make -C DP/ $@
	rm -rf *.o $(bin)

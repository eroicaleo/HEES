.PHONY : all

bin = energysys

OBJ = DCCon_out.o SuperCap.o main.o DischargeProcess.o ChargeProcess.o DCCon_in.o DCCon_dis.o LionBat.o LoadApp.o selVCTI.o \
			powersource.o DCSolver.o ees_bank.o HEESTimer.o

CC = g++
# CFLAGS = -c -Wall -I/home/yzhang/usr/sundials/include
CFLAGS = -c -Wall -g -I/home/yzhang/usr/sundials/include
LDFLAGS = -L/home/yzhang/usr/sundials/lib
# LDFLAGS = 
SUNDIALS_LIBS = -lsundials_kinsol -lsundials_nvecserial
# SUNDIALS_LIBS = 

# CFLAGS += -DDEBUG_YANGGE
# CFLAGS += -DUSING_MATLAB

all : $(bin)

$(bin) : $(OBJ)
	g++ $(LDFLAGS) $^ -o $@ $(SUNDIALS_LIBS)

%.o : %.cpp
	g++ $(CFLAGS) $< -o $@

clean :
	rm -rf *.o $(bin)

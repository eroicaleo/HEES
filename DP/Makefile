
CFLAG = -c -g -Wall
CFLAG_DP = $(CFLAG) -DDP_BINARY
EXT_OBJ = ../nnet/nnetmultitask.o ../nnet/nnet.o ../nnet/util.o \
          ../DCCon_in.o

dp = taskScheduling

dp : $(dp)_bin.o $(EXT_OBJ)
	g++ $^ -o $@

all : $(dp).o

%_bin.o : %.cpp
	g++ $(CFLAG_DP) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o dp
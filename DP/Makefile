
CFLAG = -c -g -Wall
CFLAG_DP = $(CFLAG) -DDP_BINARY

dp = taskScheduling

dp : $(dp)_bin.o
	g++ $^ -o $@

all : $(dp).o

%_bin.o : %.cpp
	g++ $(CFLAG_DP) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o dp

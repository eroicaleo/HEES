
CFLAG = -c -g -Wall
CFLAG_NNET = $(CFLAG) -DNNET_BINARY

nnet : nnet_bin.o util.o
	g++ $^ -o $@

all : nnet.o util.o

%_bin.o : %.cpp
	g++ $(CFLAG_NNET) $< -o $@

%.o : %.cpp
	g++ -c $< -o $@

util.o : util.c
	g++ -c $< -o $@

clean :
	rm -rf *.o nnet

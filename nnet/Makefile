
CFLAG = -c -g -Wall
CFLAG_NNET = $(CFLAG) -DNNET_BINARY
OBJ = util.o nnetmultitask.o

nnet : nnet_bin.o $(OBJ)
	g++ $^ -o $@

all : nnet.o $(OBJ)

%_bin.o : %.cpp
	g++ $(CFLAG_NNET) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

util.o : util.c
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o nnet

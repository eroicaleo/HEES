
CFLAG = -c -g -Wall
CFLAG_NNET = $(CFLAG) -DNNET_BINARY
CFLAG_NNET_DEBUG = $(CFLAG) -DDEBUG_NNET
OBJ = util.o nnetmultitask.o
TEST_OBJ = $(OBJ) nnet.o

TEST_DEBUG_OBJ = $(TEST_OBJ:.o=_debug.o)

nnet : nnet_bin.o $(OBJ)
	g++ $^ -o $@

all : nnet.o $(OBJ)

test : test_model_10.o $(TEST_OBJ)
	g++ test_model_10.o $(TEST_OBJ) -o test_model_10

test_debug : test_model_10.o $(TEST_DEBUG_OBJ)
	g++ test_model_10.o $(TEST_DEBUG_OBJ) -o test_model_10

%_debug.o : %.cpp
	g++ $(CFLAG_NNET_DEBUG) $< -o $@

%_bin.o : %.cpp
	g++ $(CFLAG_NNET) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

util%.o : util.c
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o nnet

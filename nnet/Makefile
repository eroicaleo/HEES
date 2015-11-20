
BOOST_INC_DIR    = "$(HOME)/usr/include"
BOOST_LIB_DIR    = "$(HOME)/usr/lib/boost"

CFLAG            = -c -g -Wall -I$(BOOST_INC_DIR)
CFLAG_NNET       = $(CFLAG) -DNNET_BINARY
CFLAG_NNET_DEBUG = $(CFLAG) -DDEBUG_NNET

OBJ      = nnetmultitask.o
TEST_OBJ = $(OBJ) nnet.o
EXT_OBJ  = ../selModel/testFilesystem.o

TEST_DEBUG_OBJ = $(TEST_OBJ:.o=_debug.o)

LDFLAG = -L$(BOOST_LIB_DIR) -lboost_system -lboost_filesystem -lboost_program_options

nnet : nnet_bin.o $(OBJ)
	g++ $^ -o $@

all : nnet.o $(OBJ)

test_multitask : test_multitask.o $(TEST_OBJ) $(EXT_OBJ)
	g++ $(LDFLAG) $^ -o $@

test_2_models : test_2_models.o $(TEST_OBJ)
	g++ test_2_models.o $(TEST_OBJ) -o test_2_models

test : test_model_10.o $(TEST_OBJ) $(EXT_OBJ)
	g++ $(LDFLAG) $^ -o test_model_10

test_debug : test_model_10.o $(TEST_DEBUG_OBJ)
	g++ test_model_10.o $(TEST_DEBUG_OBJ) -o test_model_10

%_debug.o : %.cpp
	g++ $(CFLAG_NNET_DEBUG) $< -o $@

%_bin.o : %.cpp
	g++ $(CFLAG_NNET) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o nnet

BOOST_INC_DIR    = "$(HOME)/usr/include"
BOOST_LIB_DIR    = "$(HOME)/usr/lib/boost"
CFLAGS           = -c -Wall -g -I$(BOOST_INC_DIR)


all : testFilesystem.o nnet.o
	g++ -L$(BOOST_LIB_DIR) -lboost_system -lboost_filesystem $^ -o testFilesystem

%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@

# g++ -DNNET_BINARY -DDEBUG_NNET -L$(BOOST_LIB_DIR) -I$(BOOST_INC_DIR) nnet.cpp -o nnet
# g++ selModel.cpp -o selModel
# g++ -L$(BOOST_LIB_DIR) -I$(BOOST_INC_DIR) testBLAS.cpp -o testBLAS

clean:
	rm -rf selModel testBLAS

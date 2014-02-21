
CFLAG = -c -g -Wall
CFLAG_CATS = $(CFLAG) -DCATS_BINARY

ca_ts: ca_ts_bin.o
	g++ ca_ts_bin.o -o ca_ts


ca_ts_fixed: ca_ts_fixed_bin.o
	g++ ca_ts_fixed_bin.o ../VoltageTable.o -o ca_ts_fixed

all : ca_ts.o ca_ts_fixed.o

%_bin.o : %.cpp
	g++ $(CFLAG_CATS) $< -o $@

%.o : %.cpp
	g++ $(CFLAG) $< -o $@

clean :
	rm -rf *.o ca_ts ca_ts_fixed

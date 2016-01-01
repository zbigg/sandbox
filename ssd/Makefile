
#
#
#
CXXFLAGS =  -MMD -g -Wall --std=c++11
CXXFLAGS += $(shell pkg-config tinfra --cflags)
LDFLAGS  =  -g
LDFLAGS  += $(shell pkg-config tinfra --libs)
CC = g++

ssd: ssd.o daemon.o
	$(CXX) $^ $(LIBS) $(LDFLAGS) -o $@

clean: 
	rm -rf ssd *.o *.d

-include *.d

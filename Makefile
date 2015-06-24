CC=g++

COMMON_LINK_COMPILE_FLAGS=-g -O3 -std=c++11 `pkg-config --cflags tinfra tinfra-regexp`
CXXFLAGS=-Wall -MD
LDFLAGS=
LDLIBS=`pkg-config --libs tinfra tinfra-regexp`

CXXFLAGS += $(COMMON_LINK_COMPILE_FLAGS)
LDFLAGS  += $(COMMON_LINK_COMPILE_FLAGS)

sources_with_main = $(shell ls *.cpp | xargs egrep -l "^int main")


programs = $(patsubst %.cpp,%,$(sources_with_main))

programs += atomic_segregated_memory_pool_mutex atomic_segregated_memory_atomic apfoo

programs += pe493

pe493: pe493.o xbmath/xbmath.cpp

apfoo: apfoo.o json_mo_parser.o

default: $(programs)

test_http_parser: test_http_parser.o http.o

clean:
	rm -rf $(programs) *.d

-include *.d

# jedit: :tabSize=8:mode=makefile:

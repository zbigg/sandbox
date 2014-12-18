CC=g++

COMMON_LINK_COMPILE_FLAGS=-g -std=c++11 -pthread
CXXFLAGS=-Wall -MD
LDFLAGS=
LDLIBS=-ltinfra-regexp -ltinfra-test -ltinfra -ldl -lpcre

CXXFLAGS += $(COMMON_LINK_COMPILE_FLAGS)
LDFLAGS  += $(COMMON_LINK_COMPILE_FLAGS)

sources_with_main = $(shell ls *.cpp | xargs egrep -l "^int main")


programs = $(patsubst %.cpp,%,$(sources_with_main))

programs += atomic_segregated_memory_pool_mutex atomic_segregated_memory_atomic apfoo


apfoo: apfoo.o json_mo_parser.o

default: $(programs)

test_http_parser: test_http_parser.o http.o

clean:
	rm -rf $(programs) *.d

#-include *.d

# jedit: :tabSize=8:mode=makefile:

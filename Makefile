CC=g++

COMMON_LINK_COMPILE_FLAGS=-g -std=c++11 -pthread
CXXFLAGS=-Wall -Wextra -MD
LDFLAGS=
LDLIBS=-ltinfra -ldl

CXXFLAGS += $(COMMON_LINK_COMPILE_FLAGS)
LDFLAGS  += $(COMMON_LINK_COMPILE_FLAGS)

sources_with_main = $(shell ls *.cpp | xargs egrep -l "^int main")


programs = $(patsubst %.cpp,%,$(sources_with_main))

programs += atomic_segregated_memory_pool_mutex atomic_segregated_memory_atomic

default: $(programs)


clean:
	rm -rf $(programs) *.d

#-include *.d

# jedit: :tabSize=8:mode=makefile:

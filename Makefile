CC=g++

COMMON_LINK_COMPILE_FLAGS=-g -std=c++11 -pthread
CXXFLAGS=-Wall -Wextra -MMD
LDFLAGS=
LDLIBS=-ltinfra -ldl

CXXFLAGS += $(COMMON_LINK_COMPILE_FLAGS)
LDFLAGS  += $(COMMON_LINK_COMPILE_FLAGS)

sources_with_main = $(shell ls *.cpp | xargs egrep -l "^int main")

programs = $(patsubst %.cpp,%,$(sources_with_main))

default: $(programs)


clean:
	rm -rf $(programs) *.d

# jedit: :tabSize=8:mode=makefile:

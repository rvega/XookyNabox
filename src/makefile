#
#  Makefile
#  xooky_nabox
#
#  Created by Rafael Vega on 2011-03-02.
#

# Platform specific vars
ifeq "$(shell uname)" "Darwin"
MORE_LINKER_FLAGS = -framework CoreFoundation -framework CoreAudio -framework Accelerate -L../vendor/ZenGarden/libs/Darwin-i386
else ifeq "$(shell uname)" "Linux"
MORE_LINKER_FLAGS = 
endif

# Parameters
COMPILER = g++
LINKER = g++
COMPILER_FLAGS = -Wall -g -c -I../vendor/ZenGarden/src
LINKER_FLAGS = -lsndfile -lpthread -lportaudio -lzengarden
CPP_FILES = $(wildcard *.cpp)
OBJECTS = $(notdir $(CPP_FILES:.cpp=.o)) # List of .cpp files with .o extension

# Link app
main: $(OBJECTS)
	$(LINKER) $(LINKER_FLAGS) $(MORE_LINKER_FLAGS) $(OBJECTS) -o ../bin/xooky_nabox

# Compile all cpp files into their own .o files
%.o: %.cpp
	$(COMPILER) $(COMPILER_FLAGS) $*.cpp -o $*.o

# Clean
.PHONY: clean
clean:
	-rm *.o
	-rm ../bin/*

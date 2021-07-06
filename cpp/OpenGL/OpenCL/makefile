CC=g++
AR=ar
ARFLAGS= rcs
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
	LFLAGS--lstdc++fs
	LIBS=-lOpenCL
	INCLUDES=-I.
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=
	LIBS=-framework OpenCL
	INCLUDES=-I. 
endif

SRC = OpenClContext.cpp OpenClUtils.cpp Fractal.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = libopencl.a

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


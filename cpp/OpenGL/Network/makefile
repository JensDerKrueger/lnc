CC=g++
AR=ar
ARFLAGS= rcs
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils -fopenmp
	LIBS=
	INCLUDES=-I. -I../Utils 
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp -Wno-undef-prefix
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils
	LIBS=-lomp -L ../../openmp/lib -L /opt/homebrew/lib
	INCLUDES=-I. -I../Utils -I ../../openmp/include -I /opt/homebrew/include
endif

SRC = Sockets.cpp Client.cpp Server.cpp Base64.cpp AES.cpp NetCommon.cpp StringTools.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = libnetwork.a

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


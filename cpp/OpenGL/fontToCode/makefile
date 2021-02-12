CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-fopenmp
	LIBS=
	INCLUDES=-I. -I../Utils
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-L../Network -lnetwork
	LIBS=-lomp -L ../../openmp/lib -L /opt/homebrew/lib
	INCLUDES=-I. -I ../../openmp/include -I /opt/homebrew/include -I../Utils
endif

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = font2code

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

../Utils/libutils.a:
	cd ../Utils && make

$(TARGET): $(OBJ) ../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../Utils && make clean

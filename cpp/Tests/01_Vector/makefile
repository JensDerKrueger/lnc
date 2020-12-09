CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=
	LIBS=
	INCLUDES=-I
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=
	LIBS=-lomp -L ../../openmp/lib
	INCLUDES=-I. -I ../../openmp/include 
endif

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = vectorTest

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)


$(TARGET): $(OBJ) 
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@


%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

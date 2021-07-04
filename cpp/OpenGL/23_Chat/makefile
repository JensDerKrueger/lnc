CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
	LFLAGS=-fopenmp -L../Network -lnetwork -L../Utils -lutils
	LIBS=
	INCLUDES=-I. -I../Network -I../Utils
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang
	LFLAGS=-L../Network -lnetwork -L../Utils -lutils
	LIBS=-L /opt/homebrew/lib
	INCLUDES=-I. -I /opt/homebrew/include -I../Network
endif

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = chat

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

../Network/libnetwork.a:
	cd ../Network && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../Network/libnetwork.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../Network && make clean

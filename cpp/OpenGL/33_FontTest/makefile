CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils
	LIBS=
	INCLUDES=-I. -I../Utils
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils
	LIBS=-L /opt/homebrew/lib
	INCLUDES=-I. -I../Utils -I /opt/homebrew/include
endif

SRC = main.cpp
RES = helvetica_neue.pos helvetica_neue.bmp
OBJ = $(SRC:.cpp=.o)
TARGET = fontTest

all: $(TARGET) $(RES)

$(RES):
	cp ../Utils/$@ $@

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET) $(RES)

../Utils/libutils.a:
	cd ../Utils && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) $(RES) core

mrproper: clean
	cd ../Utils && make clean

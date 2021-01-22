CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils -fopenmp
	LIBS=
	INCLUDES=-I. -I../Utils 
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils
	LIBS=-lomp -L ../../openmp/lib
	INCLUDES=-I. -I../Utils -I /opt/homebrew/include
endif

SRC = Scene.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = mirror

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
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

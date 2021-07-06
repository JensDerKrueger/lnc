CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils -fopenmp -lstdc++fs
	LIBS=-lOpenCL
	INCLUDES=-I. -I../Utils -I../OpenCL
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils -L../OpenCL -lopencl
	LIBS=-lomp -L ../../openmp/lib -L /opt/homebrew/lib -framework OpenCL
	INCLUDES=-I. -I../Utils -I../OpenCL -I /opt/homebrew/include -I ../../openmp/include
endif

SRC = LargeImageRenderer.cpp LargeImage.cpp main.cpp Fractal.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = largeImageRenderer

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

../Utils/libutils.a:
	cd ../Utils && make $(MAKECMDGOALS)

../OpenCL/libopencl.a:
	cd ../OpenCL && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../Utils/libutils.a ../OpenCL/libopencl.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../Utils && make clean
	cd ../OpenCL && make clean


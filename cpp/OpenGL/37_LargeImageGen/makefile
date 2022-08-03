CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code

OSTYPE := $(shell uname)
ifeq ($(OSTYPE),Linux)
  LIBS=-lOpenCL
  LFLAGS=-lglfw -lGLEW -lGL  -L../Utils -lutils -L../OpenCL -lopencl
  INCLUDES=-I. -I../Utils -I../OpenCL
else
  LIBS=-framework OpenCL
  LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils -L../OpenCL -lopencl -L /opt/homebrew/lib
  INCLUDES=-I. -I../Utils -I../OpenCL -I/opt/homebrew/include
endif

SRC = main.cpp MultiresGen.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = fractal

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
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


CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code

OSTYPE := $(shell uname)
ifeq ($(OSTYPE),Linux)
  LIBS=-lOpenCL
  LFLAGS=-lglfw -lGLEW -lGL  -L../Utils -lutils
  INCLUDES=-I. -I../Utils
else
  LIBS=-framework OpenCL
  LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils
  INCLUDES=-I. -I../Utils-I /opt/homebrew/include
endif

INCLUDES=-I. -I../Utils
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = compression

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

../Utils/libutils.a:
	cd ../Utils && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../Utils && make clean

CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LFLAGS= -lglfw -lglew -framework OpenGL -L../Utils -lutils 
LIBS=
INCLUDES=-I. -I../Utils
SRC = Scene.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = opengl

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

CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LFLAGS= -lglfw -lglew -framework OpenGL
LIBS=
INCLUDES=-I. 
SRC = Mat4.cpp Vec3.cpp Rand.cpp main.cpp 
OBJ = $(SRC:.cpp=.o)
TARGET = opengl

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $(LFLAGS) $(LIBS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


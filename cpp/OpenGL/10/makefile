CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LFLAGS= -lglfw -lglew -framework OpenGL -L../Utils -lutils 
LIBS=
INCLUDES=-I. -I../Utils
SRC = Scene.cpp main.cpp TextRenderer.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = tetris

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $(LFLAGS) $(LIBS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


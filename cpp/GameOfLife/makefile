CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LFLAGS=-lglfw -lGLEW -framework OpenGL
LIBS=
INCLUDES=-I.
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = gol

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

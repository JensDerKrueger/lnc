CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LIBS=
INCLUDES=-I.
SRC = main.cpp Image.cpp Fractal.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = fractal

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $(LIBS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


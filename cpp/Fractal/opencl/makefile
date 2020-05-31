CC=g++
CFLAGS=-c -Wall -std=c++0x -Wunreachable-code 
LIBS=-framework OpenCL
INCLUDES=-I.
SRC = main.cpp Image.cpp Fractal.cpp OpenClContext.cpp
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
	-cp $(TARGET) $(TARGET)_old
	-rm -rf $(OBJ) $(TARGET) core


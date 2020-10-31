CC=g++

CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils 
# for linux use this line LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils

LIBS=-lomp -L ../../openmp/lib
INCLUDES=-I. -I../Utils -I ../../openmp/include 
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = texture

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
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

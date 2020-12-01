CC=g++

CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
LFLAGS=-lglfw -lGLEW -framework OpenGL -L../LibML -lml -L../Utils -lutils
LIBS=-lomp -L ../../openmp/lib
INCLUDES=-I. -I../LibML -I../Utils -I ../../openmp/include 
SRC = main.cpp MNIST.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = digits

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

../Utils/libutils.a:
	cd ../Utils && make

../LibML/libml.a:
	cd ../LibML && make

$(TARGET): $(OBJ) ../LibML/libml.a ../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@


%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../LibML && make clean
	cd ../Utils && make clean


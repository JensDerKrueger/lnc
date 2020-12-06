CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-fopenmp
	LIBS=
	INCLUDES=-I. 
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=
	LIBS=-lomp -L ../../openmp/lib
	INCLUDES=-I. -I ../openmp/include 
endif

SRC = Rand.cpp HitableList.cpp HitRecord.cpp main.cpp Vec3.cpp Ray.cpp Camera.cpp Sphere.cpp Dielectric.cpp Metal.cpp Lambertian.cpp Mat4.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = raytrace

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $(LIBS) $(LFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


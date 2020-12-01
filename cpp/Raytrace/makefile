CC=g++
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
LFLAGS=
LIBS=-lomp -L ../openmp/lib
INCLUDES=-I.
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


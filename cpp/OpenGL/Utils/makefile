CC=g++
AR=ar
ARFLAGS= rcs
CFLAGS=-c -Wall -std=c++17 -Wunreachable-code
LIBS=
INCLUDES=-I.
SRC = Grid2D.cpp GLTexture1D.cpp Vec4.cpp FontRenderer.cpp bmp.cpp PrecomputedParticleSystem.cpp PlanarMirror.cpp FresnelVisualizer.cpp GLArray.cpp ParticleSystem.cpp AbstractParticleSystem.cpp GLTexture2D.cpp Tesselation.cpp GLBuffer.cpp GLEnv.cpp GLProgram.cpp Mat4.cpp Vec3.cpp Rand.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = libutils.a

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core


CC=g++
AR=ar
ARFLAGS= rcs
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils -fopenmp
	LIBS=
	INCLUDES=-I. -I../Utils
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils
	LIBS=-lomp -L ../../openmp/lib -L /opt/homebrew/lib
	INCLUDES=-I. -I../Utils -I ../../openmp/include -I /opt/homebrew/include
endif

SRC = \
SHA2.cpp SHA1.cpp MD5.cpp \
Image.cpp bmp.cpp OBJFile.cpp \
GLApp.cpp GLBuffer.cpp GLEnv.cpp GLProgram.cpp GLArray.cpp GLTexture2D.cpp GLTexture1D.cpp GLTexture3D.cpp GLDebug.cpp GLFramebuffer.cpp GLDepthBuffer.cpp \
ArcBall.cpp Grid2D.cpp FontRenderer.cpp PlanarMirror.cpp FresnelVisualizer.cpp Tesselation.cpp Rand.cpp DeferredShader.cpp \
ParticleSystem.cpp AbstractParticleSystem.cpp PrecomputedParticleSystem.cpp Timer.cpp

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

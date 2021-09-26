CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../../Utils -lutils -L../../Network -lutils -fopenmp
	LIBS=
	INCLUDES=-I. -I../../Utils -I../../Network -I ../../openmp/include
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-framework OpenGL -L../../Network -lnetwork -lGLEW
	LIBS=-lomp -L ../../../openmp/lib -L /opt/homebrew/lib
	INCLUDES=-I. -I ../../../openmp/include -I /opt/homebrew/include -I../../Network -I../../Utils
endif

SRC = ../PainterCommon.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = netPaintServer

all: $(TARGET)

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

../../Network/libnetwork.a:
	cd ../../Network && make $(MAKECMDGOALS)

../../Utils/libutils.a:
	cd ../../Utils && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../../Network/libnetwork.a ../../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

mrproper: clean
	cd ../../Network && make clean
	cd ../../Utils && make clean
CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -fopenmp
	LFLAGS=-lglfw -lGLEW -lGL -L../Utils -lutils -L../Network -lutils -fopenmp
	LIBS=
	INCLUDES=-I. -I../Utils -I../Network -I ../../openmp/include
else
	CFLAGS=-c -Wall -std=c++17 -Wunreachable-code -Xclang -fopenmp
	LFLAGS=-lglfw -lGLEW -framework OpenGL -L../Utils -lutils -L../Network -lnetwork
	LIBS=-lomp -L ../../openmp/lib -L /opt/homebrew/lib
	INCLUDES=-I. -I../Utils -I ../../openmp/include -I /opt/homebrew/include -I../Network
endif

SRC = DialogPhase.cpp FinishPhase.cpp MainPhase.cpp GamePhase.cpp TextPhase.cpp InputPhase.cpp BoardPhase.cpp BoardSetupPhase.cpp GameGrid.cpp ShipPlacement.cpp main.cpp BattleShips.cpp GameClient.cpp ../25_GenericGameServer/NetGame.cpp
RES = helvetica_neue.pos helvetica_neue.bmp
OBJ = $(SRC:.cpp=.o)
TARGET = battleShips

all: $(RES) $(TARGET)

$(RES):
	cp ../Utils/$@ $@

release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET) $(RES)

../Utils/libutils.a:
	cd ../Utils && make $(MAKECMDGOALS)

../Network/libnetwork.a:
	cd ../Network && make $(MAKECMDGOALS)

$(TARGET): $(OBJ) ../Network/libnetwork.a ../Utils/libutils.a
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) $(RES) core

mrproper: clean
	cd ../Network && make clean
	cd ../Utils && make clean


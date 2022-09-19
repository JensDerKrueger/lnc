CC=g++
OSTYPE := $(shell uname)

ifeq ($(OSTYPE),Linux)
	CFLAGS=-c -Wall -std=c++17
	LFLAGS=
	LIBS=
else
	CFLAGS=-c -Wall -Xclang -std=c++17 
	LFLAGS=
	LIBS=
endif

SRC = main.cpp
TARGET = c64float

OBJ = $(SRC:.cpp=.o)
all: $(TARGET)
release: CFLAGS += -O3 -Os -flto -DNDEBUG
release: LFLAGS += -flto
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $^ $(LFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm -rf $(OBJ) $(TARGET) core

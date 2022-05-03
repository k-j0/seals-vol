
OUT := seals-vol
CC := g++
CFLAGS := -O3 -std=c++17

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OUT)
	rm -f *.o

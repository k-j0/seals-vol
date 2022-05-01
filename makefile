
CFLAGS = -O3 -std=c++17

.PHONY: clean

all: seals-vol

seals-vol: main.o VolIterator.o
	g++ $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f seals-vol
	rm -f *.o

all: main

main:
	g++ -std=c++11 -o coherence gpu-cache-equal.cpp

clean:
	rm coherence

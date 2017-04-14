all: main

main:
	g++ -std=c++11 -o coherence cpu-gpu-coherence.cpp

clean:
	rm coherence

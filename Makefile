all: main

main: main.cpp
	g++ -Wall -O0 -ggdb3 $< -o $@

clean:
	rm -f main
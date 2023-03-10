all: main main_opt

main: main.cpp
	g++ -Wall -O0 -ggdb3 -static $< -o $@

main_opt: main.cpp
	g++ -Wall -msse3 -O3 -static $< -o $@

clean:
	rm -f main main_opt

all: build/main

build/main: src/main.cpp
	@mkdir -p build
	g++ -std=c++17 -Wall -Wextra -g src/main.cpp -o build/main

run: build/main
	./build/main

clean:
	rm -rf build

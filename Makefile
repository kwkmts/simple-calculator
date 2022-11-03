CXX=clang++
CXXFLAGS=-g -std=c++14 -Wall
RM=rm -rf

main: main.cpp
	$(CXX) main.cpp -o main $(CXXFLAGS)

clean:
	$(RM) main main.dSYM

.PHONY: clean

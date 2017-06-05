CXXFLAGS = -std=c++11 -Wall -Werror -O3
CXX = g++

OBJECTS = main.o rTesla.o

default: run

run: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cc rTesla.h

rTesla.o: rTesla.cc rTesla.h

clean:
	rm -f run-test *.o *~
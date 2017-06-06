CXXFLAGS = -std=c++11 -Wall -Werror -O3
CXX = g++

OBJECTS = main.o rTesla.o sha256.o

default: run

run: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cc rTesla.h

rTesla.o: rTesla.cc rTesla.h

sha256.o: sha256.cc sha256.h

clean:
	rm -f run-test *.o *~
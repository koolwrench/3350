all: hw1

hw1: hw1.cpp fonts.h
	g++ hw1.cpp libggfonts.a -Wall -o hw1 -lX11 -lGL -lGLU -lm -lrt


clean:
	rm -f hw1
	rm -f *.o


CC = g++

CFLAGS = -Wall -l gpiod

functions: main

main: main.cpp
	${CC} main.cpp ${CFLAGS}

CC=g++
CPPFLAGS= -g -Wall -std=c++11

all: trade

trade: header.o trade.cpp
		$(CC) $(CPPFLAGS) -o trade  trade.cpp header.o

header.o: header.h  header.cpp
		$(CC) $(CPPFLAGS)  header.h header.cpp
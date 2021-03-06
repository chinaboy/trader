CC=g++
CPPFLAGS= -g -Wall -std=c++11

all: trade

trade: header.o trade.cpp
		$(CC) $(CPPFLAGS) ./trade.out  -c trade.cpp

header.o: header.h  header.cpp
		$(CC) $(CPPFLAGS) ./header.o -c header.h header.cpp
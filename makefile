CC=g++

all: trade

trade: header.o trade.cpp
		$(CC) -std=c++11 -o trade  trade.cpp header.h

header.o: header.h  header.cpp
		$(CC) -std=c++11  header.h header.cpp
CC=g++

all: trade

trade: message.o header.o trade.cpp
		$(CC) -std=c++11 -o trade -c trade.cpp

message.o: message.h header.h message.cpp header.cpp
		$(CC) -std=c++11 -o message.o -c header.cpp

header.o: header.h message.h header.cpp message.cpp
		$(CC) -std=c++11 -o header.o -c message.cpp
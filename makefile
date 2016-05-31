CC=g++

all: trade

trade: message.o header.o trade.cpp
		$(CC) -std=c++11 -o trade

message.o: message.h header.h message.cpp header.cpp
		$(CC) -std=c++11 -o message

header.o: header.h message.h header.cpp message.cpp
		$(CC) -std=c++11 -o header:
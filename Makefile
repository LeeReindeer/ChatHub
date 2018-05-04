CFLAGS=-Wall -g
EVENTLIB?=-levent
REDISLIB?=-lhiredis
BINDIR?=./bin
SERVERDIR?=./server
CLIENTDIR?=./client
LIB?=./lib

all: chat 

chat: 
		gcc $(GFLAGS) -o chat $(SERVERDIR)/chat.c dao.o $(LIB)/config.o $(EVENTLIB) $(REDISLIB)
dao:
		gcc $(GFLAGS) -c -o dao.o $(SERVERDIR)/dao.c $(LIB)/config.o $(REDISLIB)
cleanall:
	rm -f *.o chart dao
clean:
	rm -f server
CFLAGS=-Wall -g
EVENTLIB?=-levent
REDISLIB?=-lhiredis
BINDIR?=./bin
SERVERDIR?=./server
CLIENTDIR?=./client
LIB?=./lib

all: config dao chat 

chat: 
		gcc $(GFLAGS) -o $(BINDIR)/chat $(SERVERDIR)/chat.c $(SERVERDIR)/dao.o $(LIB)/config.o $(EVENTLIB) $(REDISLIB)
dao:
		gcc $(GFLAGS) -c -o $(BINDIR)/dao.o $(SERVERDIR)/dao.c $(REDISLIB)
config:
		gcc -c -o $(BINDIR)/config.o $(LIB)/config.c
cleanall:
	rm -f $(BINDIR)/*.o $(BINDIR)/chat
clean:
	rm -f $(BINDIR)/chat
CFLAGS=-Wall -g
EVENTLIB?=-levent
REDISLIB?=-lhiredis
BINDIR?=./bin
SERVERDIR?=./server
CLIENTDIR?=./client
LIB?=./lib

$(shell mkdir -p $(BINDIR))

all: lib dao chat

lib: config parson message

chat: 
		gcc $(GFLAGS) -o $(BINDIR)/chat $(SERVERDIR)/chat.c $(BINDIR)/dao.o $(BINDIR)/config.o $(BINDIR)/parson.o $(BINDIR)/message.o $(EVENTLIB) $(REDISLIB)
dao:
		gcc $(GFLAGS) -c -o $(BINDIR)/dao.o $(SERVERDIR)/dao.c $(REDISLIB)
config:
		gcc -c -o $(BINDIR)/config.o $(LIB)/config.c
parson:
		gcc -c -o $(BINDIR)/parson.o $(LIB)/parson.c
message:
		gcc -c -o $(BINDIR)/message.o $(LIB)/message.c
cleanall:
	rm -f $(BINDIR)/*.o $(BINDIR)/chat
clean:
	rm -f $(BINDIR)/chat
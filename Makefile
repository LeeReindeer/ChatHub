CFLAGS=-Wall -g
EVENTLIB?=-levent
REDISLIB?=-lhiredis
BINDIR?=./bin
SERVERDIR?=./server
CLIENTDIR?=./client
LIB?=./lib

$(shell mkdir -p $(BINDIR))

all: lib dao chatserver chatclient

lib: config parson message hash

chatserver: 
		gcc $(GFLAGS) -o $(BINDIR)/chathub-server $(SERVERDIR)/server.c $(BINDIR)/dao.o $(BINDIR)/config.o $(BINDIR)/parson.o $(BINDIR)/message.o $(BINDIR)/hash.o $(EVENTLIB) $(REDISLIB)
chatclient:
		gcc $(GFLAGS) -o $(BINDIR)/chathub-cli ./client/chatcli.c ./client/net.o $(BINDIR)/config.o $(BINDIR)/parson.o $(BINDIR)/message.o
dao:
		gcc $(GFLAGS) -c -o $(BINDIR)/dao.o $(SERVERDIR)/dao.c $(REDISLIB)
config:
		gcc -c -o $(BINDIR)/config.o $(LIB)/config.c
parson:
		gcc -c -o $(BINDIR)/parson.o $(LIB)/parson.c
message:
		gcc -c -o $(BINDIR)/message.o $(LIB)/message.c
hash:
		gcc -c -o $(BINDIR)/hash.o $(LIB)/hashtable.c -lm
cleanall:
	rm -f $(BINDIR)/*.o $(BINDIR)/chathub-server
clean:
	rm -f $(BINDIR)/chathub-server
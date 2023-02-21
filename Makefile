CC = gcc
CFLAGS = -Wall -lpthread
BDIR = build
OUTPUT = chatcli

$(OUTPUT): $(BDIR)/chat_cli.o $(BDIR)/chat_server.o
	$(CC) $(CFLAGS) $(BDIR)/chat_cli.o $(BDIR)/chat_server.o  -o $(OUTPUT)

$(BDIR):
	mkdir build

$(BDIR)/chat_cli.o: chat_cli.c chat_cli.h
	$(CC) $(CFLAGS) -c chat_cli.c -o build/chat_cli.o

$(BDIR)/chat_server.o: chat_server.c chat_server.h
	$(CC) $(CFLAGS) -c chat_server.c -o build/chat_server.o

clean:
	rm $(BDIR)/*.o $(BDIR)/$(OUTPUT)
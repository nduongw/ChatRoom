all: client server
client: client.c
	gcc client.c -o client
server: server.c
	gcc server.c -o server -l sqlite3
clean:
	rm -f *.o client
	rm -f *.o server

all: client server
client: client.c
	gcc client.c -o client -pthread
server: server.c
	gcc server.c -o server -l sqlite3 -pthread
clean:
	rm -f *.o client
	rm -f *.o server
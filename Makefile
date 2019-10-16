all: server client

server: server.o socket.o
	g++ server.o socket.o -o server

server.o: server.cpp
	g++ -c -W -Wall -Wextra -std=c++11 server.cpp -o server.o

socket.o: socket.cpp
	g++ -c -W -Wall -Wextra -std=c++11 socket.cpp -o socket.o

client: client.o socket.o
	g++ client.o socket.o -o client

client.o: client.cpp
	g++ -c -W -Wall -Wextra -std=c++11 client.cpp -o client.o

clean:
	rm server.o server socket.o client.o client

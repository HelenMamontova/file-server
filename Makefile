all: server client

server: server.o socket.o
	g++ server.o socket.o -o server

server.o: server.cpp
	g++ -c server.cpp

socket.o: socket.cpp
	g++ -c socket.cpp

client: client.o socket.o
	g++ client.o socket.o -o client

client.o: client.cpp
	g++ -c client.cpp

clean:
	rm server.o server socket.o client.o client

CXXFLAGS = -W -Wall -Wextra -std=c++11

.PHONY: all clean

all: server client

server: server.o socket.o
	$(CXX) server.o socket.o -o server

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

client: client.o socket.o
	$(CXX) client.o socket.o -o client

clean:
	rm server.o server socket.o client.o client

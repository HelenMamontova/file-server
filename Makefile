CXXFLAGS = -W -Wall -Wextra -std=c++11

all: server client

server: server.o socket.o
	$(CXX) $< socket.o -o $@

server.o: server.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

socket.o: socket.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

client: client.o socket.o
	$(CXX) $< socket.o -o $@

client.o: client.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm server.o server socket.o client.o client

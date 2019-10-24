CXXFLAGS = -W -Wall -Wextra -std=c++11

.PHONY: all clean

all: server client

server: server.o socket.o
	$(CXX) $< socket.o -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

client: client.o socket.o
	$(CXX) $< socket.o -o $@

clean:
	rm server.o server socket.o client.o client

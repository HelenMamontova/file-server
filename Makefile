CXXFLAGS = -W -Wall -Wextra -std=c++11

.PHONY: all clean

all: server client

%: %.o socket.o
	$(CXX) $< socket.o -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm server.o server socket.o client.o client

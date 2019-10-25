CXXFLAGS = -W -Wall -Wextra -std=c++11

SOURCES = server.cpp \
		  client.cpp \
		  socket.cpp

.PHONY: all clean

all: server client

server: server.o socket.o
	$(CXX) $< socket.o -o $@

client: client.o socket.o
	$(CXX) $< socket.o -o $@

clean:
	rm *.o *.d server client

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@

#ifndef UTILS_H
#define UTILS_H

#include "socket.h"

#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in

bool setAddress(const std::string& address, struct sockaddr_in* local);
void sendString(Socket& sock, const std::string& source);
void receiveString(Socket& sock, std::string& destination);
void sendUint8(Socket& sock, uint8_t source);
int receiveUint8(Socket& sock, uint8_t& destination);
void sendUint32(Socket& sock, uint32_t source);
int receiveUint32(Socket& sock, uint32_t& destination);

enum Command {GET = 0, PUT = 1, LIST = 2, ERROR = 128, SUCCESS = 129, SEND_FILE = 130, SEND_LIST = 131};

#endif

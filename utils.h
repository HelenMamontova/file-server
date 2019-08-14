#ifndef UTILS_H
#define UTILS_H

#include "socket.h"

#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in

bool setAddress(const std::string& address, struct sockaddr_in* local);
void sendString(Socket& sock, const std::string& source);
std::string receiveString(Socket& sock);
void sendUint8(Socket& sock, uint8_t source);
uint8_t receiveUint8(Socket& sock);
void sendUint32(Socket& sock, uint32_t source);
uint32_t receiveUint32(Socket& sock);

enum Command {GET = 0, PUT = 1, LIST = 2, ERROR = 128, SUCCESS = 129, SEND_FILE = 130, SEND_LIST = 131};

#endif

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <netinet/in.h> //struct sockaddr_in
#include <cstdint> //uint8_t, uint32_t

bool setAddress(const std::string& address, struct sockaddr_in* local);
int sendString(int sock, const std::string& source);
int receiveString(int sock, std::string& destination);
int sendUint8(int sock, uint8_t source);
int receiveUint8(int sock, uint8_t& destination);
int sendUint32(int sock, uint32_t source);
int receiveUint32(int sock, uint32_t& destination);

enum Command {GET = 0, PUT = 1, LIST = 2, ERROR = 128, SUCCESS = 129, SEND_FILE = 130, SEND_LIST = 131};

#endif

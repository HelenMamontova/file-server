#ifndef UTILS_H
#define UTILS_H

#include "socket.h"

#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in

void makeAddress(const std::string& address, struct sockaddr_in* local);

enum Command {GET = 0, PUT = 1, LIST = 2, ERROR = 128, SUCCESS = 129, SEND_FILE = 130, SEND_LIST = 131};

#endif

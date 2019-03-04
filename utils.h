#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <netinet/in.h> //struct sockaddr_in

bool setAddress(const std::string& address, struct sockaddr_in* local);

#endif

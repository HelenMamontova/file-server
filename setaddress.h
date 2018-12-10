#include <string>
#include <netinet/in.h>

#ifndef SETADDRESS_H
#define SETADDRESS_H

bool setAddress(const std::string& address, struct sockaddr_in* local);

#endif

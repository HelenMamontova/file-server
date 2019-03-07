#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <netinet/in.h> //struct sockaddr_in

bool setAddress(const std::string& address, struct sockaddr_in* local);
int sendString(int s, const std::string& str);
int receiveString(int s, std::string& str_recv);

#endif

//
// Created by likai_m on 2018/9/13.
//

#ifndef CYAP_ALL_SOCKETDEF_H
#define CYAP_ALL_SOCKETDEF_H

#ifndef WIN32

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include<unistd.h>
#include<netinet/tcp.h>
typedef int                  socket_t;
#define INVALID_SOCKET       -1

#else

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define bzero(b, len) (void)(memset((b), '\0', (len)))
#define inet_aton(host,addr) inet_pton(AF_INET,(host),(addr))

typedef SOCKET               socket_t;
typedef int                  socklen_t;

#define close				 closesocket

#endif




#endif //CYAP_ALL_SOCKETDEF_H

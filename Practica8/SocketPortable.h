#ifndef SOCKETPORTABLE_H_INCLUDED
#define SOCKETPORTABLE_H_INCLUDED

#include <iostream>

using namespace std;

#ifdef _WIN32
/**
*   To compile in MinGW use -lws2_32 linker option
*   To compile with MSVC++ use  #pragma comment(lib,"Ws2_32.lib")
**/
#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#endif

class SocketPortable {
private:
#ifdef _WIN32
    SOCKET sockfd;
#else
    int sockfd;
#endif

public:
    SocketPortable();
    ~SocketPortable();
    string getLastErrorMessage();
    bool socket( int domain, int type, int protocol );
    bool setNonBlock();
    bool nonBlockNoError() ;
    void close();
    bool connect( const char *node, const char *service, const struct addrinfo *hints);
#ifdef _WIN32
    bool connect( const struct sockaddr *addr, int addrlen );
    int recv( char *buf, int len, int flags );
    int send( const char *buf, int len, int flags );
#else
    bool connect( const struct sockaddr *addr, socklen_t addrlen );
    ssize_t recv( void *buf, size_t len, int flags );
    ssize_t send( const void *buf, size_t len, int flags );
#endif
};

#endif // SOCKETPORTABLE_H_INCLUDED

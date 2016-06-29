#include "SocketPortable.h"

#ifdef _WIN32
SocketPortable::SocketPortable() {
    WSADATA wsaData;
    WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
}
SocketPortable::~SocketPortable() {
    closesocket( sockfd );
    WSACleanup();
}
string SocketPortable::getLastErrorMessage() {
    DWORD errorMessageID = ::GetLastError();
    if( errorMessageID == 0 )
        return ""; //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  nullptr, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )&messageBuffer, 0,
                                  nullptr );

    string message( messageBuffer, size );
    LocalFree( messageBuffer );

    return message;
}
bool SocketPortable::connect( const struct sockaddr *addr, int addrlen ) {
    if( ::connect( sockfd, addr, addrlen ) != 0 ) {
        closesocket( sockfd );
        return false;
    }
    return true;
}
bool SocketPortable::socket( int domain, int type, int protocol ) {
    sockfd = ::socket( domain, type, protocol );
    if( sockfd == INVALID_SOCKET ) {
        closesocket( sockfd );
        return false;
    }
    return true;
}
bool SocketPortable::setNonBlock() {
    u_long iMode = 0;
    if( ioctlsocket( sockfd, FIONBIO, &iMode ) != NO_ERROR ) {
        return false;
    }
    return true;
}
bool SocketPortable::nonBlockNoError() {
    return WSAGetLastError() == WSAEWOULDBLOCK;
}
void SocketPortable::close() {
    ::closesocket( sockfd );
}
int SocketPortable::recv( char *buf, int len, int flags ) {
    return ::recv( sockfd, buf, len, flags );
}
int SocketPortable::send( const char *buf, int len, int flags ) {
    return ::send( sockfd, buf, len, flags );
}
#else
SocketPortable::SocketPortable() {}
SocketPortable::~SocketPortable() {
    close();
}
string SocketPortable::getLastErrorMessage() {
    return strerror(errno);
}
bool SocketPortable::connect( const struct sockaddr *addr, socklen_t addrlen ) {
    if( ::connect( sockfd, addr, addrlen ) != 0 ) {
        close();
        return false;
    }
    return true;
}
bool SocketPortable::socket( int domain, int type, int protocol ) {
    sockfd = ::socket( domain, type, protocol );
    if( sockfd < 0 ) {
        close();
        return false;
    }
    return true;
}
bool SocketPortable::setNonBlock() {
    int flags = fcntl( sockfd, F_GETFL );
    flags = flags | O_NONBLOCK;
    if( fcntl( sockfd, F_SETFL, flags ) < 0 ) {
        return false;
    }
    return true;
}
bool SocketPortable::nonBlockNoError() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}
void SocketPortable::close() {
    ::close( sockfd );
}
ssize_t SocketPortable::recv( void *buf, size_t len, int flags ) {
    return ::recv( sockfd, buf, len, flags );
}
ssize_t SocketPortable::send( const void *buf, size_t len, int flags ) {
    return ::send( sockfd, buf, len, flags );
}
#endif
bool SocketPortable::connect( const char *node, const char *service,
                              const struct addrinfo *hints ) {
    struct addrinfo *res, *rp;

    if ( getaddrinfo( node, service, hints, &res ) != 0 ) {
        freeaddrinfo( res );
        return false;
    }

    for ( rp = res; rp != nullptr; rp = rp->ai_next ) {
        if ( !socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) {
            continue;
        }
        if ( !connect( rp->ai_addr, rp->ai_addrlen ) ) {
            continue;
        }
        break;
    }

    if ( rp == nullptr ) {
        freeaddrinfo( res );
        return false;
    }
    freeaddrinfo( res );
    return true;
}

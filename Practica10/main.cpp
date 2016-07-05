/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #10
*/
#include <iostream>
#include <cstring>
#include <cstdint>
#include <limits>

#include "SocketPortable.h"

using namespace std;

#define TAMANO_BUFFER 256

bool inputError();

int main() {
    char buffer[TAMANO_BUFFER], ip[100];
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    socklen_t addrlen;
    SocketPortable sp;
    int totalBytes;
    socklen_t optval = 1;
    uint16_t puerto;
    string grupo;

    do {
        cout << "Puerto a la escucha: ";
        cin >> puerto;
        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
    } while ( inputError() );

    do {
        cout << "Grupo multicast a suscribirse: ";
        getline( cin, grupo );
    } while ( inputError() || grupo.empty() );

    if( !sp.socket( AF_INET, SOCK_DGRAM, 0 ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    if( !sp.setsockopt( SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    addr.sin_port = htons( puerto );

    if( !sp.bind( ( struct sockaddr * ) &addr, sizeof( addr ) ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    inet_pton(AF_INET, grupo.c_str(), &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );
    if ( !sp.setsockopt( IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) < 0 ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    cout << "Esperando mensajes. . ." << endl;
    while ( true ) {
        memset( &addr, 0, sizeof( addr ) );
        addrlen = sizeof( addr );
        totalBytes = sp.recvfrom( buffer, TAMANO_BUFFER, 0, ( struct sockaddr * ) &addr, &addrlen );
        if ( totalBytes < 0 ) {
            cout << sp.getLastErrorMessage() << endl;
            return -1;
        }
        buffer[totalBytes] = 0;
        inet_ntop(AF_INET,  ( struct sockaddr * ) &addr, ip, INET_ADDRSTRLEN);
        cout << "Mensaje desde IP: " << ip  << " Puerto: " << ntohs (addr.sin_port) << endl;
        cout << buffer << endl;
    }
    return 0;
}

bool inputError() {
    if ( cin.fail() ) {
        cin.clear();
        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        return  true;
    } else {
        return  false;
    }
}

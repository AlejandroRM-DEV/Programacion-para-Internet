/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #4
*/
#include <iostream>
#include <limits>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>

#define TAMANO_BUFFER 1000

using namespace std;

bool inputError();
long long milisegundos();

int main() {
    string ip;
    int conexion, sock, flags;
    unsigned short puerto;
    struct sockaddr_in salida;
    struct hostent* resolucion;
    struct in_addr **addr_list;
    long long milisegundosActuales;
    char buffer[TAMANO_BUFFER + 1];

    do {
        cout << "Dame la direccion IP (x.x.x.x): ";
        getline( cin, ip );
    } while ( inputError() );

    do {
        cout << "Dame el puerto (0 - 65535): ";
        cin >> puerto;
    } while ( inputError() );

    resolucion = gethostbyname( ip.c_str() );
    if ( resolucion ) {
        cout << "Listado de IP" << endl;
        addr_list = ( struct in_addr ** )resolucion->h_addr_list;
        for ( int i = 0; addr_list[i] != NULL; i++ ) {
            cout << "--> " << inet_ntoa( *addr_list[i] ) << endl;
        }
        cout << endl;

        sock = socket( AF_INET, SOCK_STREAM, 0 );
        if ( sock >= 0 ) {
            cout << "INFO: Socket creado" << endl;

            salida.sin_family = AF_INET;
            salida.sin_addr.s_addr = addr_list[0]->s_addr;
            salida.sin_port = htons( puerto );

            conexion = connect( sock, ( struct sockaddr* )&salida, sizeof( salida ) );

            if ( conexion >= 0 ) {
                cout << "INFO: Conexion establecida con: " << ip
                     << " (" << inet_ntoa( salida.sin_addr ) << ") Puerto: " << puerto << endl;

                flags = fcntl(sock, F_GETFL);
                flags = flags | O_NONBLOCK;
                fcntl(sock, F_SETFL, flags);

                milisegundosActuales = milisegundos();

                while (true) {
                    if ( (milisegundos() - milisegundosActuales) >= 500 ) {
                        conexion = read( sock, buffer, TAMANO_BUFFER );
                        if ( conexion < 0 ) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                cout << ". " << std::flush;
                            } else {
                                perror( "ERROR FATAL" );
                                break;
                            }
                        } else if ( conexion == 0 ) {
                            perror( "ERROR conexion cerrada" );
                            break;
                        } else {
                            buffer[conexion] = 0;
                            cout << endl << "MENSAJE: " << endl << buffer << endl;
                        }
                        milisegundosActuales = milisegundos();
                    }
                }
            } else {
                perror( "ERROR No se pudo concretar la conexion" );
            }
            close( sock );

        } else {
            perror( "ERROR No se pudo crear el Socket" );
        }
    } else {
        perror("ERROR en el nombre host");
    }
    return 0;
}

bool inputError() {
    if ( cin.fail() ) {
        cin.clear();
        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        return true;
    } else {
        return false;
    }
}

long long milisegundos() {
    struct timeval te;
    gettimeofday(&te, nullptr);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}
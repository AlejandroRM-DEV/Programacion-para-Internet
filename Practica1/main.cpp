/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #1
*/
#include <iostream>
#include <sstream>
#include <limits>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "IPUtil.h"

#define TAMANO_BUFFER 1000

using namespace std;

bool inputError();

int main() {
    string ip;
    int conexion;
    unsigned short puerto;
    struct sockaddr_in salida;
    char buffer[TAMANO_BUFFER + 1];
    int sock = socket( AF_INET, SOCK_STREAM, 0 );

    if( sock >= 0 ) {
        cout << "INFO: Socket creado" << endl;
        do {
            cout << "Dame la direccion IP (x.x.x.x): ";
            getline( cin, ip );
        } while( inputError() || !esDireccionIPv4( ip ) );

        do {
            cout << "Dame el puerto (0 - 65535): ";
            cin >> puerto;
        } while( inputError() );

        salida.sin_family = AF_INET;
        salida.sin_addr.s_addr = htonl( transformaIPv4( ip ) );
        salida.sin_port = htons( puerto );

        conexion = connect( sock, ( struct sockaddr* )&salida, sizeof( salida ) );

        if( conexion >= 0 ) {
            cout << "INFO: Conexion establecida a IP: " << ip
                 << " (" << salida.sin_addr.s_addr << ") Puerto: " << puerto << endl;

            conexion = read( sock, buffer, TAMANO_BUFFER );
            if( conexion < 0 ) {
                cout << "ERROR FATAL" << endl;
            } else if( conexion == 0 ) {
                cout << "ERROR: conexion cerrada" << endl;
            } else {
                buffer[conexion] = 0;
                cout << "MENSAJE: "<< endl << buffer << endl;
            }
            close( sock );

        } else {
            cout << "ERROR: No se pudo concretar la conexion" << endl;
        }

    } else {
        cout << "ERROR: No se pudo crear el Socket" << endl;
    }

    return 0;
}

bool inputError() {
    if( cin.fail() ) {
        cin.clear();
        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        return true;
    } else {
        return false;
    }
}

/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #8
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <fcntl.h>
#include <sys/time.h>

#include <fstream>

#include "Lexico.h"
#include "Sintactico.h"

#define TAMANO_BUFFER 2000

long long milisegundos();
bool inputError();

int main() {
    Lexico lex;
    Sintactico sin;
    deque<ParToken> tokens;
    string str, peticion;

    cout << "Dame la URL: ";
    getline( cin, str );
    lex.reiniciar( str );
    tokens  = lex.dameListaTokens();
    if ( lex.hayError() ) {
        cout << "URL \"" << str << "\" MAL FORMADA" << endl;
        return -1;
    }
    sin.reinicia( tokens );
    URL* url = sin.analiza();
    if ( sin.hayError() ) {
        cout << "URL \"" << str << "\" MAL FORMADA" << endl;
        return -1;
    }
    if (url->protocolo == "") {
        url->protocolo = "http";
    } else if (url->protocolo != "http"  && url->protocolo != "HTTP") {
        cout << "Protocolo no soportado, saliendo..." << endl;
        return -1;
    }
    if (url->puerto == "" ) {
        url->puerto = "80";
    }
    if (url->ruta == "" ) {
        url->ruta = "/";
    }

    peticion = "GET " + url->ruta + " HTTP/1.1" +
               "\r\nHost: " + url->host +
               "\r\nConnection: close\r\n\r\n";

    /*****************************************/

    int conexion, sockfd, flags;
    char buffer[TAMANO_BUFFER + 1];
    struct addrinfo hints;
    struct addrinfo *resultado, *rp;
    long long milisegundosActuales;

    memset(&hints, 0, sizeof (addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    conexion = getaddrinfo(url->host.c_str(), url->puerto.c_str(), &hints, &resultado);

    if ( conexion != 0) {
        perror("ERROR en el nombre host");
        return -1;
    }

    for (rp = resultado; rp != nullptr; rp = rp->ai_next ) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if ( sockfd < 0 ) {
            perror( "ERROR No se pudo crear el Socket" );
            close(sockfd);
            continue;
        }
        conexion = connect( sockfd, rp->ai_addr, rp->ai_addrlen );
        if ( conexion < 0 ) {
            perror( "ERROR No se pudo concretar la conexion" );
            close(sockfd);
            continue;
        }
        cout << "INFO: conexion establecida" << endl;
        break;
    }

    if (rp == nullptr) {
        perror("No se puedo conectar a ninguna direccion del host");
        return -1;
    }

    freeaddrinfo(resultado);

    flags = fcntl(sockfd, F_GETFL);
    flags = flags | O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    write(sockfd, peticion.c_str(), strlen(peticion.c_str()));

    milisegundosActuales = milisegundos();

    fstream ss("temp.dat", fstream::in | fstream::out | fstream::binary | fstream::trunc);
    while (true) {
        if ( (milisegundos() - milisegundosActuales) >= 500 ) {
            conexion = read( sockfd, buffer, TAMANO_BUFFER );
            if ( conexion < 0 ) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    cout << ". " << std::flush;
                } else {
                    perror( "ERROR FATAL" );
                    break;
                }
            } else if ( conexion == 0 ) {
                cout << "Conexion cerrada" << endl;
                break;
            } else {
                buffer[conexion] = 0;
                ss.write((char*)&buffer, conexion);
            }
            milisegundosActuales = milisegundos();
        }
    }
    close( sockfd );

    ss.clear();
    ss.seekg( 0, ios::beg );

    string respuesta = "";
    bool finHead = false;
    size_t pos;
    do {
        ss.read( (char*) &buffer , sizeof(buffer) );
        string s(buffer);
        pos = s.find("\r\n\r\n");
        if (pos == string::npos) {
            respuesta += s;
        } else {
            respuesta += s.substr(0, pos);
            finHead = true;
        }
    } while (!finHead);

    ss.clear();
    ss.seekg( pos + 4, ios::beg ); // +4 "\r\n\r\n"

    char resp;
    string aux, codigo, codigoDesc, archivoNombre;

    istringstream issRespuesta (respuesta);
    issRespuesta >> aux >> codigo >> codigoDesc;

    if (codigo == "200") {
        do {
            cout << "Desea guardar el archivo? (S/N): ";
            cin >> resp;
        } while (inputError());
        if (resp == 'S' || resp == 's') {
            cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            do {
                cout << "Ingresa el nombre del archivo: ";
                getline(cin, archivoNombre);
            } while (inputError());

            cout << "Guardando. . . " << endl;
            ofstream file(archivoNombre.c_str(), ios::binary);
            if (file.good()) {
                file << ss.rdbuf();
                file.close();
            }
        } else {
            cout << "Saliendo. . ." << endl;
        }
    } else {
        cout << "ERROR: " << codigo << " " << codigoDesc << endl;
    }
    ss.close();

    /*****************************************/

    return 0;
}

long long milisegundos() {
    struct timeval te;
    gettimeofday(&te, nullptr);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
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
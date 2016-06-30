#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/time.h>

#include "SocketPortable.h"
#include "URLParser.h"

using namespace std;

#define TAMANO_BUFFER 2000

long long milisegundos();
bool inputError();
URL* obtenerURL();
bool realizarPeticionHTTP( URL* url, fstream& temp );
void procesarRespuestaHTTP( fstream& temp );

int main() {
    URL* url = obtenerURL();
    if ( url == nullptr ) {
        return -1;
    }

    fstream temp( "tempbuf.dat", fstream::in | fstream::out | fstream::binary | fstream::trunc );
    if ( temp.is_open() && temp.good() ) {
        if( realizarPeticionHTTP( url, temp ) ) {
            procesarRespuestaHTTP( temp );
        }
        temp.close();
    } else {
        cout << "Error al crear el archivo temporal \"tempbuf.dat\"" << endl;
        return -1;
    }

    return 0;
}

URL* obtenerURL() {
    string str, peticion;

    cout << "Dame la URL: ";
    getline( cin, str );

    URL* url = URLParser::parse( str );
    if ( url == nullptr ) {
        cout << "URL \"" << str << "\" MAL FORMADA" << endl;
        return nullptr;
    }
    if ( url->protocolo.empty() ) {
        url->protocolo = "http";
    } else if ( url->protocolo != "http"  && url->protocolo != "HTTP" ) {
        cout << "Protocolo no soportado, saliendo..." << endl;
        return nullptr;
    }
    if ( url->puerto.empty() ) {
        url->puerto = "80";
    }
    if ( url->ruta.empty() ) {
        url->ruta = "/";
    }
    return url;
}

bool realizarPeticionHTTP( URL* url, fstream& temp ) {
    string peticion = "GET " + url->ruta + " HTTP/1.1" +
                      "\r\nHost: " + url->host +
                      "\r\nConnection: close\r\n\r\n";

    struct addrinfo hints;
    long long milisegundosActuales;
    char buffer[TAMANO_BUFFER + 1];
    int totalBytes;
    SocketPortable sp;

    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( !sp.connect( url->host.c_str(), url->puerto.c_str(), &hints ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return false;
    }
    if( !sp.setNonBlock() ) {
        cout << sp.getLastErrorMessage() << endl;
        return false;
    }
    cout << "Socket listo." << endl << "Enviando peticion. . ." << endl;
    sp.send( peticion.c_str(), strlen( peticion.c_str() ), 0 );

    milisegundosActuales = milisegundos();
    cout << "Esperando respuesta. . ." << endl;
    while ( true ) {
        if ( ( milisegundos() - milisegundosActuales ) >= 500 ) {
            totalBytes = sp.recv( buffer, TAMANO_BUFFER, 0 );
            if ( totalBytes < 0 ) {
                if ( sp.nonBlockNoError() ) {
                    cout << ". " << std::flush;
                } else {
                    perror( "ERROR FATAL" );
                    return false;
                }
            } else if ( totalBytes == 0 ) {
                cout << "Conexion cerrada" << endl;
                break;
            } else {
                buffer[totalBytes] = 0;
                temp.write( ( char* )&buffer, totalBytes );
            }
            milisegundosActuales = milisegundos();
        }
    }
    sp.close();
    return true;
}

void procesarRespuestaHTTP( fstream& temp ) {
    string respuesta, aux, codigo, codigoDesc, archivoNombre;
    char buffer[TAMANO_BUFFER + 1];
    char resp;
    size_t pos;
    bool finHead = false;

    temp.clear();
    temp.seekg( 0, ios::beg );
    do {
        temp.read( ( char* ) &buffer, sizeof( buffer ) );
        aux = buffer ;
        pos = aux.find( "\r\n\r\n" );
        if ( pos == string::npos ) {
            respuesta += aux;
        } else {
            respuesta += aux.substr( 0, pos );
            finHead = true;
        }
    } while ( !finHead );

    temp.clear();
    temp.seekg( pos + 4, ios::beg ); // +4 "\r\n\r\n"

    istringstream issRespuesta ( respuesta );
    issRespuesta >> aux >> codigo >> codigoDesc;

    if ( codigo == "200" ) {
        do {
            cout << "Desea guardar el archivo? (S/N): ";
            cin >> resp;
        } while ( inputError() );
        if ( resp == 'S' || resp == 's' ) {
            cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            do {
                cout << "Ingresa el nombre del archivo: ";
                getline( cin, archivoNombre );
            } while ( inputError() );

            cout << "Guardando. . . " << endl;
            ofstream ofs( archivoNombre.c_str(), ios::binary );
            if ( ofs.is_open() && ofs.good() ) {
                ofs << temp.rdbuf();
                ofs.close();
            } else {
                cout << "No es posible guardar el archivo" << endl;
            }
        } else {
            cout << "Saliendo. . ." << endl;
        }
    } else {
        cout << "ERROR: " << codigo << " " << codigoDesc << endl;
    }
}

long long milisegundos() {
    struct timeval te;
    gettimeofday( &te, nullptr );
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

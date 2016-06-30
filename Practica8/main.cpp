/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #8
*/
#include <iostream>
#include <sstream>
#include <cstdio>
#include <sys/time.h>

#include "SocketPortable.h"
#include "URLParser.h"

using namespace std;

#define TAMANO_BUFFER 2048

long long milisegundos();
bool inputError();
URL* obtenerURL();
bool realizarPeticionHTTP( URL* url, FILE * temp );
void procesarRespuestaHTTP( FILE * temp );

//http://entropymag.org/wp-content/uploads/2014/10/outer-space-wallpaper-pictures.jpg
int main() {
    // fopen, fread, fwrite para mantener compatibilidad con windows
    FILE * temp;
    URL* url = obtenerURL();
    if ( url == nullptr ) {
        return -1;
    }
    temp = fopen ( "tempbuf.dat", "w+b" );
    if ( temp == NULL ) {
        cout << "Error al crear el archivo temporal \"tempbuf.dat\" necesario" << endl;
        return -1;
    }
    if( realizarPeticionHTTP( url, temp ) ) {
        procesarRespuestaHTTP( temp );
        fclose ( temp );
        return 0;
    } else {
        fclose ( temp );
        return -1;
    }
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

bool realizarPeticionHTTP( URL* url, FILE * temp ) {
    long long milisegundosActuales;
    char buffer[TAMANO_BUFFER + 1];
    struct addrinfo hints;
    int totalBytes;
    SocketPortable sp;
    string peticion = "GET " + url->ruta + " HTTP/1.1" +
                      "\r\nHost: " + url->host +
                      "\r\nConnection: close\r\n\r\n";

    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( !sp.connect( url->host.c_str(), url->puerto.c_str(), &hints ) && !sp.setNonBlock() ) {
        cout << sp.getLastErrorMessage() << endl;
        return false;
    }

    cout << "Socket listo." << endl << "Enviando peticion. . ." << endl;
    sp.send( peticion.c_str(), peticion.length(), 0 );

    cout << "Recibiendo respuesta. . ." << endl;
    totalBytes = -1;
    milisegundosActuales = milisegundos();
    while ( totalBytes != 0 ) {
        if ( ( milisegundos() - milisegundosActuales ) >= 50 ) {
            totalBytes = sp.recv( ( char* )&buffer, TAMANO_BUFFER, 0 );
            if ( totalBytes < 0 ) {
                if ( !sp.nonBlockNoError() ) {
                    perror( "ERROR FATAL" );
                    return false;
                }
            } else if ( totalBytes > 0 ) {
                buffer[totalBytes] = 0;
                fwrite( buffer, sizeof( char ), totalBytes, temp );
            }
            cout << ". " << std::flush;
            milisegundosActuales = milisegundos();
        }
    }
    cout << "Conexion cerrada" << endl;
    sp.close();
    fflush( temp );
    return true;
}

void procesarRespuestaHTTP( FILE * temp ) {
    string respuesta, aux, codigo, descripcionCodigo, archivoNombre;
    size_t offsetTemp, offsetArchivo;
    char buffer[TAMANO_BUFFER];
    char guardar;
    FILE * archivo;
    bool finHead = false;

    fseek( temp, 0, SEEK_SET );
    do {
        fread( buffer, sizeof( char ), TAMANO_BUFFER, temp );
        aux = buffer;
        offsetTemp = aux.find( "\r\n\r\n" );
        if ( offsetTemp == string::npos ) {
            respuesta += aux;
        } else {
            respuesta += aux.substr( 0, offsetTemp );
            finHead = true;
        }
    } while ( !finHead );

    fseek( temp, offsetTemp + 4, SEEK_SET ); // +4 ( \r\n\r\n )

    istringstream issRespuesta ( respuesta );
    issRespuesta >> aux >> codigo >> descripcionCodigo;

    if ( codigo == "200" ) {
        do {
            cout << "Desea guardar el archivo? (S/N): ";
            cin >> guardar;
            cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        } while ( inputError() || ( guardar != 'S' && guardar != 's' && guardar != 'N' &&
                                    guardar != 'n' ) );
        if ( guardar == 'S' || guardar == 's' ) {
            do {
                cout << "Ingresa el nombre del archivo: ";
                getline( cin, archivoNombre );
            } while ( inputError() || archivoNombre.empty() );

            cout << "Guardando. . . " << endl;
            archivo = fopen ( archivoNombre.c_str(), "wb" );
            if ( archivo != NULL ) {
                offsetArchivo = fread( buffer, sizeof( char ), TAMANO_BUFFER, temp );
                while( offsetArchivo == TAMANO_BUFFER ) {
                    fwrite( buffer, sizeof( char ), TAMANO_BUFFER, archivo );
                    offsetArchivo = fread( buffer, sizeof( char ), TAMANO_BUFFER, temp );
                }
                fwrite( buffer, sizeof( char ), offsetArchivo, archivo );
                fclose ( archivo );
            } else {
                cout << "No es posible guardar el archivo" << endl;
            }
        } else {
            cout << "Saliendo. . ." << endl;
        }
    } else {
        cout << "ERROR: " << codigo << " " << descripcionCodigo << endl;
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

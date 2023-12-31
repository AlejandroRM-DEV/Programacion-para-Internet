/*
*   Alejandro Ramirez Mu�oz
*   213496617
*   Programacion para Internet - 2016V
*   Practica Listado FTP
*/
#include <iostream>
#include <sstream>
#include <cstring>

#include "SocketPortable.h"
#include "URLParser.h"

#define TAMANO_BUFFER 512
#define NIVEL_PROFUNDIDAD 3

#define RESPUESTA_INESPERADA -2

#define C_STR(ITEMS)  ( ( dynamic_cast<ostringstream &> ( ostringstream() . seekp( 0, ios_base::cur ) << ITEMS ) ) \
                       .str().c_str() )

using namespace std;

static bool LOG = true;

template <typename T>
inline bool str_to_num( const string& str, T &num ) {
    istringstream iss( str );
    return ( iss >> num ) ? true : false;
}

bool inputError();
URL* obtenerURL();
int recibir( SocketPortable &socket, string &copia );
int recibir( SocketPortable &control );
int listarDirectorios( SocketPortable &control, int profundidad,  stringstream &listado, string padding ) throw ( int );

int main() {
    stringstream listado;
    string comando;
    int codigo;
    struct addrinfo hints;
    SocketPortable control;

    URL* url = obtenerURL();
    if ( url == nullptr ) {
        return -1;
    }

    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    try {
        if( !control.connect( url->host.c_str(), url->puerto.c_str(), &hints ) ) throw - 1;
        if( !( codigo = recibir( control ) ) ) throw codigo;
        switch( codigo ) {
        case 120: if( !( codigo = recibir( control ) ) ) throw codigo; break;
        case 220: break; // Todos bien
        case 421: throw RESPUESTA_INESPERADA;
        }

        comando = C_STR( "USER " << url->usuario << "\r\n" );
        if( !( codigo = control.send( comando.c_str(), comando.length(), 0 ) ) ) throw codigo;
        if( !( codigo = recibir( control ) ) ) throw codigo;
        switch( codigo ) {
        case 530: case 500: case 501: case 421: case 332: throw RESPUESTA_INESPERADA;
        case 230: case 331: break; // Todos bien
        }

        comando = C_STR( "PASS " << url->contrasena << "\r\n" );
        if( !( codigo = control.send( comando.c_str(), comando.length(), 0 ) ) )throw codigo;
        if( !( codigo = recibir( control ) ) ) throw codigo;
        switch( codigo ) {
        case 230: break; // Todos bien
        case 202: case 530: case 500: case 501: case 503: case 421: case 332: throw RESPUESTA_INESPERADA;
        }

        if( !( codigo = control.send( "CWD /\r\n", 7, 0 ) ) ) throw codigo;
        if( !( codigo = recibir( control ) ) ) throw codigo;
        switch( codigo ) {
        case 250: break; // Todos bien
        case 500: case 501: case 502: case 421: case 530: case 550: throw RESPUESTA_INESPERADA;
        }

        listado << "---| Root" << endl;
        listarDirectorios( control, 0, listado, "   |" );
        cout << listado.str() << endl;

        if( !( codigo = control.send( "QUIT\r\n", 6, 0 ) ) ) throw codigo;
        if( !( codigo = recibir( control ) ) ) throw codigo;
        switch( codigo ) {
        case 221: break; // Todos bien
        case 500: throw RESPUESTA_INESPERADA;
        }
    } catch( int e ) {
        if( e == -1 ) {
            cout << control.getLastErrorMessage() << endl;
            control.close();
            return -1;
        } else if( e == 0 ) {
            cout << "Conexion cerrada" << endl;
            control.close();
        } else if( e == RESPUESTA_INESPERADA ) {
            cout << "La aplicacion se cerro, no era posible continuar la ejecucion" << endl;
            control.close();
        }
    }

    return 0;
}

URL* obtenerURL() {
    string str;

    cout << "Dame la URL: ";
    getline( cin, str );

    URL* url = URLParser::parse( str );
    if ( url == nullptr ) {
        cout << "URL \"" << str << "\" MAL FORMADA" << endl;
        return nullptr;
    }
    if ( url->protocolo.empty() ) {
        url->protocolo = "ftp";
    } else if ( url->protocolo != "ftp"  && url->protocolo != "FTP" ) {
        cout << "Protocolo no soportado, saliendo..." << endl;
        return nullptr;
    }
    if ( url->puerto.empty() ) {
        url->puerto = "21";
    }
    if ( url->ruta.empty() ) {
        url->ruta = "/";
    }
    if( url->usuario.empty() ) {
        cout << "Por favor ingresa un usuario: ";
        getline( cin, url->usuario );
    }
    if( url->contrasena.empty() ) {
        cout << "Por favor ingresa una contrasena: ";
        getline( cin, url->contrasena );
    }
    return url;
}

int recibir( SocketPortable &control ) {
    string respuesta;
    int resultado = recibir( control, respuesta );
    if( LOG ) cout << respuesta;
    return resultado;
}

int recibir( SocketPortable &socket, string &copia ) {
    char *buffer = new char[TAMANO_BUFFER + 1];
    int resultado;

    copia = "";
    do {
        resultado = socket.recv( buffer, TAMANO_BUFFER, 0 );
        if( resultado == 0 ) {
            return 0;
        } else if ( resultado < 0 ) {
            return -1;
        } else {
            buffer[resultado] = 0;
            copia += buffer;
        }
    } while( resultado == TAMANO_BUFFER );
    str_to_num( C_STR( copia.at( 0 ) << copia.at( 1 ) << copia.at( 2 ) ), resultado );

    delete buffer;
    return resultado;
}

int listarDirectorios( SocketPortable &control, int profundidad, stringstream &listado, string padding )throw ( int ) {
    stringstream ssPasivo, ssListado;
    string aux, aux_2, ip, nombre;
    SocketPortable datos;
    struct addrinfo hints;
    uint16_t puerto, temp;
    size_t inicio;
    int codigo;

    if( !( codigo = control.send( "PASV\r\n", 6, 0 ) ) ) throw codigo;
    if( !( codigo = recibir( control, aux ) ) ) throw codigo;
    switch( codigo ) {
    case 227: break; // Todos bien
    case 500: case 501: case 502: case 421: case 530: throw RESPUESTA_INESPERADA;
    }
    if( LOG ) cout << aux;

    inicio = aux.find_first_of( "(", 0 ) + 1;
    ssPasivo << aux.substr( inicio, aux.find_first_of( ")", 0 ) - inicio );

    getline( ssPasivo, aux_2, ',' );
    ip = aux_2 + ".";
    getline( ssPasivo, aux_2, ',' );
    ip += aux_2 + ".";
    getline( ssPasivo, aux_2, ',' );
    ip += aux_2 + ".";
    getline( ssPasivo, aux_2, ',' );
    ip += aux_2;

    getline( ssPasivo, aux_2, ',' );
    str_to_num( aux_2, puerto );
    getline( ssPasivo, aux_2 );
    str_to_num( aux_2, temp );
    puerto = ( puerto * 256 ) + temp;

    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if( !datos.connect( ip.c_str(), C_STR( puerto ), &hints ) ) {
        cout << datos.getLastErrorMessage() << endl;
        datos.close();
        return -1;
    }

    if( !( codigo = control.send( "LIST\r\n", 6, 0 ) ) ) throw codigo;
    if( !( codigo = recibir( control ) ) ) throw codigo;
    switch( codigo ) {
    case 125: case 150: break;
    case 450: case 500: case 501: case 502: case 421: case 530: throw RESPUESTA_INESPERADA;
    }
    if( recibir( datos, aux ) < 0 ) {
        cout << datos.getLastErrorMessage() << endl;
        datos.close();
        return -1;
    }
    datos.close();
    if( !( codigo = recibir( control ) ) ) throw codigo;
    switch( codigo ) {
    case 226: case 250: break; // Todo bien
    case 425: case 426: case 421: throw RESPUESTA_INESPERADA;
    }

    ssListado << aux ;
    while( getline( ssListado, aux_2 ) ) {
        istringstream issfila( aux_2 );
        issfila >> aux >> aux >> aux >> aux >> aux >> aux >> aux >> aux >> nombre;

        listado << padding;
        if( aux_2.at( 0 ) != 'd' ) {
            listado << "--->> " << nombre << endl;
        } else {
            listado << "---[] " << nombre << endl;

            if( profundidad < NIVEL_PROFUNDIDAD ) {
                if( !( codigo = control.send( "PWD\r\n", 5, 0 ) ) ) throw codigo;
                if( !( codigo = recibir( control, aux ) ) ) throw codigo;
                switch( codigo ) {
                case 257: break; // Todos bien
                case 500: case 501: case 502: case 421: case 550: throw RESPUESTA_INESPERADA;
                }
                aux.erase( 0, aux.find_first_of( "\"" ) + 1 );
                aux.erase( aux.find_first_of( "\"" ) );

                aux_2 = C_STR( "CWD " << aux << ( aux.back() == '/' ? "" : "/" ) << nombre << "\r\n" );
                if( !( codigo = control.send( aux_2.c_str(), aux_2.length(), 0 ) ) ) throw codigo;
                if( !( codigo = recibir( control ) ) ) throw codigo;
                switch( codigo ) {
                case 250: break; // Todos bien
                case 500: case 501: case 502: case 421: case 530: case 550: throw RESPUESTA_INESPERADA;
                }

                listarDirectorios( control, profundidad + 1, listado, ( padding +  "   |"  ) );

                if( !( codigo = control.send( "CDUP\r\n", 6, 0 ) ) ) throw codigo;
                if( !( codigo = recibir( control ) ) ) throw codigo;
                switch( codigo ) {
                case 200: break; // Todos bien
                case 500: case 501: case 502: case 421: case 530: case 550: throw RESPUESTA_INESPERADA;
                }

                listado << padding << endl;
            }
        }
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

/*
*   Alejandro Ramirez Muñoz
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

using namespace std;

#define C_STR(ITEMS)  ( ( dynamic_cast<ostringstream &> ( ostringstream() . seekp( 0, ios_base::cur ) << ITEMS ) ) \
                       .str().c_str() )

template <typename T>
inline bool str_to_num( const std::string& str, T &num ) {
    std::istringstream iss( str );
    return ( iss >> num ) ? true : false;
}

bool inputError();
URL* obtenerURL();
int recibir( SocketPortable &control, string &copiaDescripcion );
int recibir( SocketPortable &control );
int listarDirectorios( SocketPortable &control, int profundidad,  stringstream &listado );

int listarDirectorios( SocketPortable &control, int profundidad, stringstream &listado ) {
    string descripcion, str, ip, aux, nombre;
    uint16_t puerto, temp;
    SocketPortable datos;

    if( control.send( "PASV\r\n", 6, 0 ) < 0) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control, descripcion ) <= 0 ) return -1;

    size_t inicio = descripcion.find_first_of( "(", 0 ) + 1;
    stringstream ss( descripcion.substr( inicio, descripcion.find_first_of( ")", 0 ) - inicio ) );

    getline( ss, str, ',' );
    ip = str + ".";
    getline( ss, str, ',' );
    ip += str + ".";
    getline( ss, str, ',' );
    ip += str + ".";
    getline( ss, str, ',' );
    ip += str;

    getline( ss, str, ',' );
    str_to_num( str, puerto );
    getline( ss, str );
    str_to_num( str, temp );
    puerto = ( puerto * 256 ) + temp;

    struct addrinfo hints;
    /*  Connection Establishment  */
    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if( !datos.connect( ip.c_str(), C_STR( puerto ), &hints ) ) {
        cout << datos.getLastErrorMessage() << endl;
        return -1;
    }

    if( control.send( "LIST\r\n", 6, 0 )< 0 ) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;

    descripcion = "";
    if( recibir( datos, descripcion ) < 0 ) return -1;
    datos.close();
    if( recibir( control ) <= 0 ) return -1;

    stringstream ss_2( descripcion );
    while( getline( ss_2, str ) ) {
        istringstream fila( str );
        fila >> aux >> aux >> aux >> aux >> aux >> aux >> aux >> aux >> nombre;
        for( int i = 0, j = profundidad + 1; i < j; i++ ) {
            listado << "    |";
        }
        if( str.at( 0 ) != 'd' ) {
            listado << "---> " << nombre << endl;
        } else {
            listado << "---[ " << nombre << endl;

            if( profundidad < 2 ) {
                if( control.send( "PWD\r\n", 5, 0 )< 0 ) {
                    cout << control.getLastErrorMessage() << endl;
                    return -1;
                }
                if( recibir( control, aux ) <= 0 ) return -1;
                aux.erase( 0, aux.find_first_of( "\"" ) + 1 );
                aux.erase( aux.find_first_of( "\"" ) );

                string send = C_STR( "CWD " << aux << ( aux.back() == '/' ? "" : "/" ) << nombre << "\r\n" );
                if( control.send( send.c_str(), send.length(), 0 )< 0 ) {
                    cout << control.getLastErrorMessage() << endl;
                    return -1;
                }
                if( recibir( control ) <= 0 ) return -1;

                listarDirectorios( control, profundidad + 1, listado );

                if( control.send( "CDUP\r\n", 6, 0 )< 0 ) {
                    cout << control.getLastErrorMessage() << endl;
                    return -1;
                }
                if( recibir( control ) <= 0 ) return -1;
            }
        }
    }
    for( int i = 0, j = profundidad; i < j; i++ ) {
        listado << "    |";
    }
    listado << endl;
    return 0;
}

int main() {
    struct addrinfo hints;
    SocketPortable control;

    URL* url = obtenerURL();
    if ( url == nullptr ) {
        return -1;
    }
    /*  Connection Establishment  */
    memset( &hints, 0, sizeof ( addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if( !control.connect( url->host.c_str(), url->puerto.c_str(), &hints ) ) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;
    /*  Usuario  */
    if( control.send( C_STR( "USER " << url->usuario << "\r\n" ), 7 + url->usuario.length(), 0 ) < 0  ) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;
    /*  Contraseña  */
    if( control.send( C_STR( "PASS " << url->contrasena << "\r\n" ), 7 + url->contrasena.length(), 0 ) < 0  ) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;
    /*  Listado  */
    if( control.send( "CWD /\r\n", 7, 0 ) < 0) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;
    stringstream listado;
    listado << "---| Root" << endl;
    listarDirectorios( control, 0, listado );
    cout << listado.str() << endl;
    /*  Salir  */
    if( control.send( "QUIT\r\n", 6, 0 ) < 0 ) {
        cout << control.getLastErrorMessage() << endl;
        return -1;
    }
    if( recibir( control ) <= 0 ) return -1;

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
    string nulo;
    return recibir( control, nulo );
}

int recibir( SocketPortable &control, string &copiaDescripcion ) {
    char *buffer = new char[TAMANO_BUFFER + 1];
    int codigo, res;

    do {
        res = control.recv( buffer, TAMANO_BUFFER, 0 );
        if( res == 0 ) {
            //cout << "Conexion cerrada" << endl;
            codigo = 0;
        } else if ( res < 0 ) {
            cout << control.getLastErrorMessage() << endl;
            codigo = -1;
        } else {
            buffer[res] = 0;
            //cout << buffer;
            copiaDescripcion += buffer;
        }
    } while( res == TAMANO_BUFFER );
    str_to_num( C_STR( copiaDescripcion.at( 0 ) << copiaDescripcion.at( 1 ) << copiaDescripcion.at( 2 ) ), codigo );

    delete buffer;
    return codigo;
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

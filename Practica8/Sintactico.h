#ifndef SINTACTICO_H_INCLUDED
#define SINTACTICO_H_INCLUDED


class URL {
public:
    string protocolo;
    string host;
    string puerto;
    string ruta;
    string usuario;
    string contrasena;

    URL() {
        protocolo = "";
        host = "";
        puerto = "";
        ruta = "";
        usuario = "";
        contrasena = "";
    }
    ~URL() {}
};

class Sintactico {
private:
    bool error;
    deque<ParToken> tokens;
    void comprueba( string simbolo ) {
        if ( !tokens.empty() && tokens.front().first == simbolo ) {
            tokens.pop_front();
        } else {
            marcarError ( );
        }
    }
    void comprueba( Token tipo ) {
        if ( !tokens.empty() && tokens.front().second == tipo ) {
            tokens.pop_front();
        } else {
            marcarError ( );
        }
    }
    void marcarError() {
        error = true;
    }

public:
    Sintactico() {}
    ~Sintactico() {}
    bool hayError() {
        return error;
    }

    void reinicia( deque<ParToken> tokens ) {
        error = false;
        this->tokens = tokens;
    }
    URL* analiza() {
        URL* url = new URL();
        init_url( url );

        return ( !hayError() ) ? url : nullptr;
    }

    void init_url( URL* url ) {
        auto dospuntos = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == ":";
        } );
        if( dospuntos != tokens.end() ) {
            auto barrauno = next( dospuntos );
            if( barrauno != tokens.end() && ( *barrauno ).first == "/" ) {
                auto barrados = next( dospuntos );
                if( barrados != tokens.end() && ( *barrados ).first == "/" ) {
                    esquema( url );
                    comprueba( ":" );
                    comprueba( "/" );
                    comprueba( "/" );
                }
            }
        }

        login( url );
        if ( !tokens.empty() ) {
            comprueba( "/" );
            rutaurl( url );
        }
    }
    void esquema( URL* url ) {
        while ( !tokens.empty() ) {
            if ( letradigito() || tokens.front().first == "+" ||
                    tokens.front().first == "-" || tokens.front().first == "." ) {
                url->protocolo += tokens.front().first;
                tokens.pop_front();
            } else {
                break;
            }
        }
    }

    /**************************************************************
    *   Partes de esquemas de URL para protocolos basados en ip   *
    **************************************************************/
    void login( URL* url ) {
        auto arroba = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == "@";
        } );
        auto barra = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == "/";
        } );
        /**
            Si hay un "@" antes de una "/" (si existe) la URL tiene usuario
        */
        if ( distance( tokens.begin(), arroba ) < distance( tokens.begin(), barra ) ) {
            url->usuario = usuario();
            if ( tokens.front().first == ":" ) {
                tokens.pop_front();
                url->contrasena = contrasena();
            }
            comprueba( "@" );
        }
        if( !hayError() && !tokens.empty() ) {
            puertomaquina( url );
        } else {
            error = true;
        }
    }

    void puertomaquina( URL* url ) {
        maquina( url );
        if ( tokens.front().first == ":" ) {
            tokens.pop_front();
            url->puerto = tokens.front().first;
            comprueba( DIGITO );
        }
    }

    void maquina( URL* url ) {
        auto letra = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.second == MINUSCULA || element.second == MAYUSCULA;
        } );
        auto barra = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == "/";
        } );
        /**
            nombremáquina() obliga la existencia de almenos UNA LETRA, si esta existe
            en la parte "host", usar dicha regla
        */
        if ( distance( tokens.begin(), letra ) < distance( tokens.begin(), barra ) ) {
            url->host = nombremaquina();
        } else {
            url->host = numeromaquina();
        }
    }

    string nombremaquina() {
        string nombre = "" ;
        auto punto = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == ".";
        } );
        auto barra = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == "/";
        } );

        /**
            *[ etiquetadominio "." ]
        **/
        while ( punto != tokens.end() &&
                ( distance( tokens.begin(), punto ) < distance( tokens.begin(), barra ) ) ) {
            nombre += etiquetadominio();
            nombre += tokens.front().first;
            comprueba( "." );
            punto = find_if( tokens.begin(), tokens.end(),
            []( const pair<string, int>& element )->bool{
                return element.first == ".";
            } );
        }
        nombre += etiquetasup();
        return nombre;
    }

    string etiquetadominio() {
        string dominio = "";
        auto punto = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.first == ".";
        } );

        if ( letradigito() ) {
            dominio += tokens.front().first;
            tokens.pop_front();
            etiquetacomun( punto );
        } else {
            marcarError();
        }
        return dominio;
    }

    string etiquetasup() {
        string sup = "";
        auto reservado = find_if( tokens.begin(), tokens.end(),
        []( const pair<string, int>& element )->bool{
            return element.second == RESERVADO;
        } );

        if ( letra() ) {
            sup += tokens.front().first;
            tokens.pop_front();
            etiquetacomun( reservado );
        } else {
            marcarError();
        }
        return sup;
    }

    string etiquetacomun( deque<ParToken>::iterator paro ) {
        string str = "";
        if ( distance( tokens.begin(), paro ) >= 1 ) {
            /**
                *[ letradígito | "-" ] letradígito
            **/
            while ( distance( tokens.begin(), paro ) > 1 ) {
                if ( letradigito() || tokens.front().first == "-" ) {
                    str += tokens.front().first;
                    tokens.pop_front();
                } else {
                    marcarError();
                }
            }
            if ( letradigito() ) {
                str += tokens.front().first;
                tokens.pop_front();
            } else {
                marcarError();
            }
        }
        return str;
    }

    string numeromaquina() {
        string ip = "";
        ip = tokens.front().first;
        comprueba( DIGITO );
        ip += tokens.front().first;
        comprueba( "." );
        ip += tokens.front().first;
        comprueba( DIGITO );
        ip += tokens.front().first;
        comprueba( "." );
        ip += tokens.front().first;
        comprueba( DIGITO );
        ip += tokens.front().first;
        comprueba( "." );
        ip += tokens.front().first;
        comprueba( DIGITO );
        return ip;
    }

    string usuario() {
        string usuario = "", s = tokens.front().first;
        while ( s == ";" || s == "?" || s == "&" || s == "=" || uchar() ) {
            usuario += s;
            tokens.pop_front();
            s = tokens.front().first;
        }
        return usuario;
    }

    string contrasena() {
        string contrasena = "", s = tokens.front().first;
        while ( s == ";" || s == "?" || s == "&" || s == "=" || uchar() ) {
            contrasena += s;
            tokens.pop_front();
            s = tokens.front().first;
        }
        return contrasena;
    }
    // depende del protocolo ver sección 3.1 del documento RFC 1738
    void rutaurl( URL* url ) {
        url->ruta = "/";
        while ( !tokens.empty() ) {
            if ( xchar() ) {
                url->ruta += tokens.front().first;
                tokens.pop_front();
            } else {
                marcarError();
                break;
            }
        }
    }
    /***********************************
    *       Definiciones varias        *
    ***********************************/
    bool letra() {
        return tokens.front().second == MINUSCULA || tokens.front().second == MAYUSCULA;
    }

    bool letradigito() {
        return tokens.front().second == DIGITO || letra();
    }

    bool noreservado() {
        return letra() || tokens.front().second == DIGITO || tokens.front().second == SEGURO ||
               tokens.front().second == EXTRA;
    }

    bool uchar() {
        return tokens.front().second == ESCAPE || noreservado ();
    }

    bool xchar() {
        return tokens.front().second == RESERVADO || uchar ();
    }

};

#endif // SINTACTICO_H_INCLUDED

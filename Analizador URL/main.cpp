#include <iostream>
#include <algorithm>
#include <deque>

using namespace std;

typedef pair<string, int> ParToken;

enum Token {
    MINUSCULA, MAYUSCULA, DIGITO, SEGURO, EXTRA, NACIONAL, PUNTUACION, RESERVADO, ESCAPE, ERROR
};

class Lexico {
private:
    enum Estado {
        Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, Q10, Q11, K, NUMERO_ESTADOS
    };
    enum Entrada {
        E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, OTRO, NUMERO_ENTRADAS
    };
    enum Salida {
        NO, SI
    };
    int matriz[NUMERO_ESTADOS][NUMERO_ENTRADAS] = {
        {Q1, Q1, Q2, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, K},
        {Q1, Q1, K, K, K, K, K, K, K, K, K, K},
        {K, K, Q2, Q2, K, K, K, K, K, K, K, K},
        {K, K, K, K, Q3, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {Q10, K, Q10, K, Q10, K, K, K, K, K, K, K},
        {Q11, K, Q10, K, Q10, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K},
        {K, K, K, K, K, K, K, K, K, K, K, K}
    };
    int salidas[NUMERO_ESTADOS][NUMERO_ENTRADAS] = {
        {NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO},
        {NO, NO, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, NO, NO, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, NO, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {NO, SI, NO, SI, NO, SI, SI, SI, SI, SI, SI, SI},
        {NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO},
        {SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI, SI},
        {NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO}
    };

    int estado;
    Token tipo;
    bool error;
    string simbolo;
    string url;

    void fijaTipo( int estado  ) {
        switch ( estado ) {
        case Q1:
            tipo = MINUSCULA;
            break;
        case Q2:
            tipo = MAYUSCULA;
            break;
        case Q3:
            tipo = DIGITO;
            break;
        case Q4:
            tipo = SEGURO;
            break;
        case Q5:
            tipo = EXTRA;
            break;
        case Q6:
            tipo = NACIONAL;
            break;
        case Q7:
        case Q9:
            tipo = PUNTUACION;
            break;
        case Q8:
            tipo = RESERVADO;
            break;
        case Q10:
            tipo = ESCAPE;
            break;
        default:
            tipo = ERROR;
            break;
        }
    }
    Entrada transicion( char c ) {
        if ( c >= 'a' && c <= 'f' ) {
            return E0;
        } else  if ( c >= 'g' && c <= 'z' ) {
            return E1;
        } else if ( c >= 'A' && c <= 'F' ) {
            return E2;
        } else  if ( c >= 'G' && c <= 'Z' ) {
            return E3;
        } else if ( c >= '0' && c <= '9' ) {
            return E4;
        } else if ( c == '$' || c == '-' || c ==  '_' || c == '.' || c == '+' ) {
            return E5;
        } else if ( c == '!' || c == '*' || c == '\'' || c == '(' || c == ')' || c == ',' ) {
            return E6;
        } else if ( c == '{' || c == '}' || c == '|' || c == 92 || c == 94 || c == 126 || c == 91 ||
                    c == 93 || c == 96 ) {
            /**
            *   91 [    92 \    93 ]    94 ^    96 `    126 ~
            */
            return E7;
        } else if ( c == '<' || c == '>' || c == '#' || c == '"' ) {
            return E8;
        } else if ( c == ';' || c == '/' || c == '?' || c == ':' || c == '@' || c == '&' || c == '=' ) {
            return E9;
        } else if ( c == '%' ) {
            return E10;
        } else {
            return OTRO;
        }
    }
public:
    Lexico( ) {
    }

    ~Lexico() {}

    void reiniciar( string url ) {
        this->url = url;
        reverse( this->url.rbegin(), this->url.rend() );
        error = false;
        simbolo = "";
        tipo = ERROR;
    }
    void marcarError() {
        cout << "ERROR Lexico: " << simbolo << endl;
        error = true;
    }
    void sigSimbolo() {
        int columna;

        estado = Q0;
        simbolo = "";
        error = false;

        while ( !fin() ) {
            columna = transicion( ( char )url.back() );
            if ( salidas[estado][columna] == SI ) {
                break;
            }
            estado = matriz[estado][columna];
            simbolo += url.back();
            url.pop_back();

            if ( estado == K ) {
                marcarError(  );
                break;
            }
        }
        fijaTipo( estado );
    }
    bool hayError() {
        return error;
    }
    bool fin() {
        return url.empty() || hayError();
    }
    string dameSimbolo() {
        return simbolo;
    }
    int dameTipo() {
        return tipo;
    }
    deque<ParToken> dameListaTokens() {
        deque<ParToken> listaTokens;
        while ( !fin() ) {
            sigSimbolo();
            listaTokens.push_back( make_pair( simbolo, tipo ) );
        }
        return listaTokens;
    }
};

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
        if ( tokens.front().first == simbolo ) {
            tokens.pop_front();
        } else {
            marcarError ( );
        }
    }
    void comprueba( Token tipo ) {
        if ( tokens.front().second == tipo ) {
            tokens.pop_front();
        } else {
            marcarError ( );
        }
    }
    void marcarError() {
        cout << "ERROR Sintactico: " << tokens.front().first << endl;
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
        esquema(url);
        comprueba( ":" );
        comprueba( "/" );
        comprueba( "/" );
        login( url );
        if ( !tokens.empty() ) {
            comprueba( "/" );
            rutaurl(url);
        }
    }
    void esquema(URL* url) {
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
        puertomaquina( url );
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

int main() {
    Lexico lex;
    Sintactico sin;
    deque<ParToken> tokens;
    string str ;

    cout << "Dame la URL: ";
    getline(cin, str);
    cout << str << endl;
    lex.reiniciar( str );
    tokens  = lex.dameListaTokens();
    if ( !lex.hayError() ) {
        sin.reinicia( tokens );
        URL* url = sin.analiza();
        if ( !sin.hayError() ) {
            cout << "URL Valida" << endl;
            cout << "Protocolo: " << url->protocolo << endl;
            cout << "Host: " << url->host << endl;
            cout << "Puerto: " << url->puerto << endl;
            cout << "Ruta: " << url->ruta << endl;
            cout << "Usuario: " << url->usuario << endl;
            cout << "Contrasena: " << url->contrasena << endl;
        } else {
            cout << "URL \"" << str << "\" MAL FORMADA" << endl;
        }
    } else {
        cout << "URL \"" << str << "\" MAL FORMADA" << endl;
    }

    return 0;
}

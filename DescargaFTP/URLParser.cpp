#include "URLParser.h"

URL* URLParser::parse( string strurl ) {
    Lexico lex;
    Sintactico sin;

    lex.reiniciar( strurl );
    if ( lex.hayError() ) {
        return nullptr;
    }

    sin.reinicia( lex.dameListaTokens() );
    return sin.analiza();
}
/*==================================================================================================
                                        LEXICAL ANALYSIS
==================================================================================================*/
URLParser::Lexico::Lexico() {}
URLParser::Lexico::~Lexico() {}

void URLParser::Lexico::fijaTipo( int estado  ) {
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
        tipo = T_ERROR;
        break;
    }
}
URLParser::Lexico::Entrada URLParser::Lexico::transicion( char c ) {
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

void URLParser::Lexico::reiniciar( string url ) {
    this->url = url;
    reverse( this->url.rbegin(), this->url.rend() );
    error = false;
    simbolo = "";
    tipo = T_ERROR;
}
void URLParser::Lexico::marcarError() {
    error = true;
}
void URLParser::Lexico::sigSimbolo() {
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
bool URLParser::Lexico::hayError() {
    return error;
}
bool URLParser::Lexico::fin() {
    return url.empty() || hayError();
}
string URLParser::Lexico::dameSimbolo() {
    return simbolo;
}
int URLParser::Lexico::dameTipo() {
    return tipo;
}
deque<URLParser::ParToken> URLParser::Lexico::dameListaTokens() {
    deque<ParToken> listaTokens;
    while ( !fin() ) {
        sigSimbolo();
        listaTokens.push_back( make_pair( simbolo, tipo ) );
    }
    return listaTokens;
}
/*==================================================================================================
                                        SYNTACTIC ANALYSIS
==================================================================================================*/
URLParser::Sintactico::Sintactico() {}
URLParser::Sintactico::~Sintactico() {}

void URLParser::Sintactico::comprueba( string simbolo ) {
    if ( !tokens.empty() && tokens.front().first == simbolo ) {
        tokens.pop_front();
    } else {
        marcarError ( );
    }
}
void URLParser::Sintactico::comprueba( Token tipo ) {
    if ( !tokens.empty() && tokens.front().second == tipo ) {
        tokens.pop_front();
    } else {
        marcarError ( );
    }
}
void URLParser::Sintactico::marcarError() {
    error = true;
}

bool URLParser::Sintactico::hayError() {
    return error;
}

void URLParser::Sintactico::reinicia( deque<ParToken> tokens ) {
    error = false;
    this->tokens = tokens;
}
URL* URLParser::Sintactico::analiza() {
    URL* url = new URL();
    init_url( url );

    return ( !hayError() ) ? url : nullptr;
}

void URLParser::Sintactico::init_url( URL* url ) {
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
void URLParser::Sintactico::esquema( URL* url ) {
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
void URLParser::Sintactico::login( URL* url ) {
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

void URLParser::Sintactico::puertomaquina( URL* url ) {
    maquina( url );
    if ( tokens.front().first == ":" ) {
        tokens.pop_front();
        url->puerto = tokens.front().first;
        comprueba( DIGITO );
    }
}

void URLParser::Sintactico::maquina( URL* url ) {
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

string URLParser::Sintactico::nombremaquina() {
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

string URLParser::Sintactico::etiquetadominio() {
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

string URLParser::Sintactico::etiquetasup() {
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


string URLParser::Sintactico::etiquetacomun( deque<ParToken>::iterator paro ) {
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

string URLParser::Sintactico::numeromaquina() {
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

string URLParser::Sintactico::usuario() {
    string usuario = "", s = tokens.front().first;
    while ( s == ";" || s == "?" || s == "&" || s == "=" || uchar() ) {
        usuario += s;
        tokens.pop_front();
        s = tokens.front().first;
    }
    return usuario;
}

string URLParser::Sintactico::contrasena() {
    string contrasena = "", s = tokens.front().first;
    while ( s == ";" || s == "?" || s == "&" || s == "=" || uchar() ) {
        contrasena += s;
        tokens.pop_front();
        s = tokens.front().first;
    }
    return contrasena;
}
// depende del protocolo ver sección 3.1 del documento RFC 1738
void URLParser::Sintactico::rutaurl( URL* url ) {
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
bool URLParser::Sintactico::letra() {
    return tokens.front().second == MINUSCULA || tokens.front().second == MAYUSCULA;
}

bool URLParser::Sintactico::letradigito() {
    return tokens.front().second == DIGITO || letra();
}

bool URLParser::Sintactico::noreservado() {
    return letra() || tokens.front().second == DIGITO || tokens.front().second == SEGURO ||
           tokens.front().second == EXTRA;
}

bool URLParser::Sintactico::uchar() {
    return tokens.front().second == ESCAPE || noreservado ();
}

bool URLParser::Sintactico::xchar() {
    return tokens.front().second == RESERVADO || uchar ();
}

#ifndef LEXICO_H_INCLUDED
#define LEXICO_H_INCLUDED

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


#endif // LEXICO_H_INCLUDED

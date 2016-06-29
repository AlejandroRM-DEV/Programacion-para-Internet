#ifndef URLPARSER_H_INCLUDED
#define URLPARSER_H_INCLUDED

#include <iostream>
#include <algorithm>
#include <deque>

using namespace std;

class URL {
public:
    string protocolo;
    string host;
    string puerto;
    string ruta;
    string usuario;
    string contrasena;

    URL() {}
    ~URL() {}
};

class URLParser {
public:
    static URL* parse( string strurl );
private:
    typedef pair<string, int> ParToken;

    URLParser() {}

    enum Token {
        MINUSCULA, MAYUSCULA, DIGITO, SEGURO, EXTRA, NACIONAL, PUNTUACION, RESERVADO, ESCAPE, T_ERROR
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
        bool error;
        Token tipo;
        string url;
        string simbolo;

        void fijaTipo( int estado  );
        Entrada transicion( char c );
    public:
        Lexico();
        ~Lexico();
        void reiniciar( string url );
        void marcarError();
        void sigSimbolo();
        bool hayError();
        bool fin();
        string dameSimbolo();
        int dameTipo();
        deque<ParToken> dameListaTokens();
    };

    class Sintactico {
    private:
        bool error;
        deque<ParToken> tokens;

        void comprueba( string simbolo );
        void comprueba( Token tipo ) ;
        void marcarError();
    public:
        Sintactico();
        ~Sintactico();
        bool hayError();
        void reinicia( deque<ParToken> tokens );
        URL* analiza();
        void init_url( URL* url );
        void esquema( URL* url );
        void login( URL* url );
        void puertomaquina( URL* url ) ;
        void maquina( URL* url );
        string nombremaquina();
        string etiquetadominio();
        string etiquetasup();
        string etiquetacomun( deque<ParToken>::iterator paro );
        string numeromaquina();
        string usuario();
        string contrasena();
        void rutaurl( URL* url );
        bool letra();
        bool letradigito();
        bool noreservado();
        bool uchar();
        bool xchar();
    };
};

#endif // URLPARSER_H_INCLUDED

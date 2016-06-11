#ifndef IPUTIL_H_INCLUDED
#define IPUTIL_H_INCLUDED

#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

unsigned int transformaIPv4( string str );
bool esDireccionIPv4( string ip );
vector<string> octetosIPv4( string s );
bool esNumeroPositivo( string s );

unsigned int transformaIPv4( string str ) {
    char punto;
    int byte[4];
    unsigned int ip;
    istringstream s( str );

    s >> byte[0] >> punto >> byte[1] >> punto >> byte[2] >> punto >> byte[3];
    ip = ( byte[0] << 24 ) | ( byte[1] << 16 ) | ( byte[2] << 8 ) | byte[3];

    return ip;
}

bool esDireccionIPv4( string ip ) {
    int oct;
    vector<string> octetosIP;

    if( ip.length() ) {
        octetosIP = octetosIPv4( ip );
        if( octetosIP.size() == 4 ) {
            for( string str : octetosIP ) {
                if( !esNumeroPositivo( str ) ) {
                    return false;
                }
                istringstream iss( str );
                iss >> oct;
                /*
                 *   iss.fail() opcional
                 *   !esNumeroPositivo(str) asegura que sean solo numeros y no deberia haber errores
                 */
                if( oct < 0 || oct > 255 ) {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}

vector<string> octetosIPv4( string ip ) {
    vector<string> octetos;
    stringstream ss( ip );
    string oct;

    while( getline( ss, oct, '.' ) ) {
        octetos.push_back( oct );
    }

    return octetos;
}

bool esNumeroPositivo( string s ) {
    return !s.empty() && all_of( s.begin(), s.end(), ::isdigit );
}

#endif // IPUTIL_H_INCLUDED

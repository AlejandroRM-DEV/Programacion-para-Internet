/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #6
*/
#include <iostream>
#include <limits>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <fcntl.h>
#include <sys/time.h>

#define TAMANO_BUFFER 1000

using namespace std;

bool inputError();
long long milisegundos();

int main() {
	string ip, puerto;
	int conexion, sockfd, flags;
	char buffer[TAMANO_BUFFER + 1];
	struct addrinfo hints;
	struct addrinfo *resultado, *rp;
	long long milisegundosActuales;
	char strAddr[100];

	do {
		cout << "Dame la direccion IP: ";
		getline( cin, ip );
	} while ( inputError() );

	do {
		cout << "Dame el puerto: ";
		getline( cin, puerto );
	} while ( inputError() );

	memset(&hints, 0, sizeof (addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	conexion = getaddrinfo(ip.c_str(), puerto.c_str(), &hints, &resultado);

	if ( conexion != 0) {
		perror("ERROR en el nombre host");
		return 1;
	}

	for (rp = resultado; rp != nullptr; rp = rp->ai_next ) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if ( sockfd < 0 ) {
			perror( "ERROR No se pudo crear el Socket" );
			close(sockfd);
			continue;
		}

		switch (rp->ai_family) {
		case AF_INET:
			cout << "INFO: intentando conectar a IP: " <<
			     inet_ntop(AF_INET, &(( (struct sockaddr_in *)rp->ai_addr )->sin_addr), strAddr, INET_ADDRSTRLEN) << endl;
			break;
		case AF_INET6:
			cout << "INFO: intentando conectar a IP: " <<
			     inet_ntop(AF_INET6, &(( (struct sockaddr_in6 *)rp->ai_addr )->sin6_addr), strAddr, INET6_ADDRSTRLEN) << endl;
			break;
		default:
			break;
		}

		conexion = connect( sockfd, rp->ai_addr, rp->ai_addrlen );
		if ( conexion < 0 ) {
			perror( "ERROR No se pudo concretar la conexion" );
			close(sockfd);
			continue;
		}
		cout << "INFO: conexion establecida" << endl;
		break;
	}

	if (rp == nullptr) {
		perror("No se puedo conectar a ninguna");
		return 1;
	}

	freeaddrinfo(resultado);

	flags = fcntl(sockfd, F_GETFL);
	flags = flags | O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

	milisegundosActuales = milisegundos();

	while (true) {
		if ( (milisegundos() - milisegundosActuales) >= 500 ) {
			conexion = read( sockfd, buffer, TAMANO_BUFFER );
			if ( conexion < 0 ) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					cout << ". " << std::flush;
				} else {
					perror( "ERROR FATAL" );
					break;
				}
			} else if ( conexion == 0 ) {
				cout << "Conexion cerrada" << endl;
				break;
			} else {
				buffer[conexion] = 0;
				cout << endl << "MENSAJE: " << endl << buffer << endl;
			}
			milisegundosActuales = milisegundos();
		}
	}
	close( sockfd );

	return 0;
}

bool inputError() {
	if ( cin.fail() ) {
		cin.clear();
		cin.ignore( numeric_limits<streamsize>::max(), '\n' );
		return true;
	} else {
		return false;
	}
}

long long milisegundos() {
	struct timeval te;
	gettimeofday(&te, nullptr);
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
	return milliseconds;
}

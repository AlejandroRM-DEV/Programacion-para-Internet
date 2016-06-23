/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #7
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
#include <poll.h>

#ifdef _WIN32
#define CLEAR "cls"
#elif __linux__
#define CLEAR "clear"
#else
#define CLEAR ""
#endif

#define TAMANO_BUFFER 1000
#define TAMANO_STR 100
#define LEER 1
#define ENVIAR 2
#define SALIR 3

using namespace std;

bool inputError();
void menu();
void enviar(int sockfd);
void leer(int sockfd);

int main() {
	string puerto;
	int conexion, sockfd, opcion;
	struct addrinfo hints;
	struct addrinfo *resultado, *rp;

	do {
		cout << "Dame el puerto: ";
		getline( cin, puerto );
	} while ( inputError() );

	memset(&hints, 0, sizeof (addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;

	conexion = getaddrinfo("::0", puerto.c_str(), &hints, &resultado);

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

		conexion = bind(sockfd, (struct sockaddr *)rp->ai_addr, rp->ai_addrlen);
		if ( conexion < 0 ) {
			perror( "ERROR en bind" );
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

	do {
		system(CLEAR);
		menu();
		cin >> opcion;
		switch (opcion) {
		case LEER:
			leer( sockfd );
			break;
		case ENVIAR:
			enviar( sockfd );
			break;
		case SALIR:
			close( sockfd );
			break;
		default: break;
		}
	} while (opcion != SALIR);


	return 0;
}


void enviar(int sockfd) {
	string host, puerto, mensaje;
	struct addrinfo hints;
	struct addrinfo *resultado;

	cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
	do {
		cout << "Dame el host: ";
		getline( cin, host );
	} while ( inputError() );

	do {
		cout << "Dame el puerto: ";
		getline( cin, puerto );
	} while ( inputError() );

	do {
		cout << "Dame el mensaje: ";
		getline( cin, mensaje );
	} while ( inputError() );

	memset(&hints, 0, sizeof (addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ( getaddrinfo(host.c_str(), puerto.c_str(), &hints, &resultado) != 0) {
		perror("ERROR host inalcanzable");
		return;
	}
	cout << "Enviando. . ." << endl;
	sendto(sockfd, mensaje.c_str(), mensaje.size(), 0, (struct sockaddr *)resultado->ai_addr,  resultado->ai_addrlen);
	freeaddrinfo(resultado);
}

void leer(int sockfd) {
	char strAddr[TAMANO_STR + 1], buffer[TAMANO_BUFFER + 1];
	int total;
	struct sockaddr_storage origen;
	socklen_t tamano;
	struct pollfd pfd;

	pfd.fd = sockfd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	system(CLEAR);
	cout << "Esperando mensajes. . ." << endl;
	do {
		if ( poll(&pfd, 1, 1000) > 0) {
			memset(&origen, 0, sizeof (sockaddr));
			tamano = sizeof(origen);
			total = recvfrom(pfd.fd, buffer, TAMANO_BUFFER, 0, (struct sockaddr *)&origen, &tamano);
			if ( total < 0 ) {
				perror( "Error al recibir el mensaje" );
			} else {
				buffer[total] = 0;
				cout << endl << "Recibiendo desde. . ." << endl;
				switch (origen.ss_family) {
				case AF_INET:
					cout << "IP: " <<  inet_ntop(AF_INET,  &(((struct sockaddr_in *)&origen)->sin_addr),  strAddr, INET_ADDRSTRLEN)
					     << " Puerto: " <<  ntohs (((struct sockaddr_in *)&origen)->sin_port) << endl;
					break;
				case AF_INET6:
					cout << "IP: " <<  inet_ntop(AF_INET6,  &(((struct sockaddr_in6 *)&origen)->sin6_addr),  strAddr, INET6_ADDRSTRLEN)
					     << " Puerto: " <<  ntohs (((struct sockaddr_in6 *)&origen)->sin6_port) << endl;
					break;
				default:
					break;
				}
				cout << "Mensaje: " << buffer << endl;
			}
		}
		cout << "." << std::flush;
	} while (strcmp(buffer, "QUIT") != 0);
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

void menu() {
	cout << "1) Leer" << endl;
	cout << "2) Enviar" << endl;
	cout << "3) Salir" << endl;
	cout << "Opcion: ";
}
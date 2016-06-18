/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #5
*/
#include <iostream>
#include <limits>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>

using namespace std;

#define TAMANO_BUFFER 1000

bool inputError();

int main() {
	int conexion, sockfd, cliente_len, cant, resRead;
	unsigned short puerto;
	struct sockaddr_in servidor, cliente;
	struct pollfd fds[50];
	char buffer[TAMANO_BUFFER + 1];

	do {
		cout << "Dame el puerto (0 - 65535): ";
		cin >> puerto;
	} while ( inputError() );

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )  {
		perror( "ERROR No se pudo crear el Socket" );
		return 1;
	}
	cout << "INFO: Socket creado" << endl;

	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = INADDR_ANY;
	servidor.sin_port = htons( puerto );

	conexion = bind( sockfd, ( struct sockaddr* )&servidor, sizeof( servidor ) );
	if ( conexion < 0 ) {
		perror( "ERROR No se crear el Socket en el puerto" );
		return 1;
	}

	cout << "Escuchando en el puerto: " << puerto << endl;
	conexion = listen(sockfd, 5);

	cout << "Esperando clientes. . ." << endl;
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	cant = 1;

	do {
		conexion = poll(fds, cant, 50);
		if (conexion < 0 ) {
			perror("Error en POll");
			break;
		} else if (conexion > 0) {
			if ( (fds[0].revents & POLLIN) == POLLIN ) {
					cliente_len = sizeof(cliente);
				fds[cant].fd = accept(sockfd,  (struct sockaddr *)&cliente, (socklen_t*)&cliente_len);
				cout << "Nuevo cliente IP: " << inet_ntoa(cliente.sin_addr) << " Puerto: " << ntohs (cliente.sin_port)
				     << " fd: " << fds[cant].fd << endl << endl;
				fds[cant].events = POLLIN;
				fds[cant].revents = 0;
				cant++;
			}
			for (int i = 1; i < cant; i++) {
				if ( (fds[i].revents & POLLIN) == POLLIN ) {
					resRead = read(fds[i].fd, buffer, TAMANO_BUFFER );
					if ( resRead < 0 ) {
						perror( "ERROR FATAL" );
						cout << "El cliente (fd: " << fds[i].fd << ") ha sido cerrado" << endl;
						close(fds[i].fd);
						fds[i] = fds[cant - 1];
						i--; // El ultimo esta en la posicion actual ahora, atender desde ahi
						cant --;
					} else if ( resRead == 0 ) {
						cout << "El cliente (fd: " << fds[i].fd << ") ha sido cerrado" << endl;
						close(fds[i].fd);
						fds[i] = fds[cant - 1];
						i--; // El ultimo esta en la posicion actual ahora, atender desde ahi
						cant --;
					} else {
						buffer[resRead] = 0;
						cout << endl << "fd: " << fds[i].fd << " MENSAJE: " << endl << buffer << endl << endl;
					}
				}
			}
		} 	// "else" No ocurrio ningun evento
	} while (true);
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
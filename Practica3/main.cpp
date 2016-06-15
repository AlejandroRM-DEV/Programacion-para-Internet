/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica #3
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

using namespace std;

bool inputError();

int main() {
	int conexion, sockfd, clientefd, cliente_len;
	unsigned short puerto;
	struct sockaddr_in servidor, cliente;

	do {
		cout << "Dame el puerto (0 - 65535): ";
		cin >> puerto;
	} while ( inputError() );

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd >= 0 ) {
		cout << "INFO: Socket creado" << endl;

		servidor.sin_family = AF_INET;
		servidor.sin_addr.s_addr = INADDR_ANY;
		servidor.sin_port = htons( puerto );

		conexion = bind( sockfd, ( struct sockaddr* )&servidor, sizeof( servidor ) );

		if ( conexion >= 0 ) {
			cout << "Escuchando en el puerto: " << puerto << endl;
			conexion = listen(sockfd, 5);
			cliente_len = sizeof(cliente);
			cout << "Esperando clientes. . ." << endl;
			do {
				clientefd = accept(sockfd,  (struct sockaddr *)&cliente, (socklen_t*)&cliente_len);
				cout << "Cliente IP: " << inet_ntoa(cliente.sin_addr) << " Puerto: " << cliente.sin_port << endl;
				cout << "Enviando mensaje. . ." << endl;
				write(clientefd, "Hola", 4);
				close(clientefd);
			} while (true);
		} else {
			perror( "ERROR No se pudo concretar la conexion" );
		}
		close( sockfd );
	} else {
		perror( "ERROR No se pudo crear el Socket" );
	}
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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define RANDOM(x) ((int) (x ## .0 * rand () / (RAND_MAX + 1.0)))


long timevaldiff (struct timeval *starttime, struct timeval *finishtime) {
  long msec;
  msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
  msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
  return msec;
}

int main (int argc, char *argv[]) {
	srand((unsigned int)time(NULL));
	int c;
	int g, res, r;
	
	char hostname[100];
	int puerto;
	long diff;
	char descartar [2000];
	
	struct timeval inicio, fin;
	
	printf ("Nombre del host a conectar: ");
	fgets (hostname, 100, stdin);
	if (hostname[strlen(hostname) - 1] == '\n') hostname[strlen (hostname) - 1] = 0;
	
	printf ("Puerto a conectar: ");
	scanf ("%i", &puerto);
	
	c = socket (AF_INET, SOCK_STREAM, 0);
	if (c < 0) {
		perror ("Error al crear el socket");
		return EXIT_FAILURE;
	}
	
	struct sockaddr_in destino;
	destino.sin_family = AF_INET;
	destino.sin_port = htons (puerto);
	
	/* Resolución de nombres */
	struct hostent *resolv;
	
	resolv = gethostbyname (hostname);
	
	if (resolv == NULL) {
		perror ("Error en la resolución de nombres");
		close (c);
		return EXIT_FAILURE;
	}
	
	memcpy (&destino.sin_addr, resolv->h_addr_list[0], resolv->h_length);
	
	res = connect (c, (struct sockaddr *) &destino, sizeof (struct sockaddr_in));
	
	if (res < 0) {
		perror ("Falló la conexión");
		close (c);
		return EXIT_FAILURE;
	}
	
	/* Aplicar no bloqueante */
	int flags;
	
	flags = fcntl (c, F_GETFL);
	flags = flags | O_NONBLOCK;
	fcntl (c, F_SETFL, flags);
	
	g = 0;
	r = RANDOM(10) + 2;
	printf ("Tiempo: %i\n", r);
	gettimeofday (&inicio, NULL);
	
	while (g < 3) {
		gettimeofday (&fin, NULL);
		
		diff = timevaldiff (&inicio, &fin);
		res = read (c, descartar, 2000);
		
		if (res > 0) {
			printf ("Llegaron datos y se descartaron\n");
		} else if (res == 0) {
			printf ("Cierre de conexión\n");
			break;
		} else {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				printf ("Error de socket\n");
				break;
			}
		}
		
		if (diff >= (r * 1000)) {
			/* Envia el mensaje */
			if (g == 0) {
				write (c, "Holis", 5);
			} else if (g == 1) {
				write (c, "Podrías aprobar la materia si trabajas mas", strlen ("Podrías aprobar la materia si trabajas mas"));
			} else if (g == 2) {
				write (c, "Suerte con tu horario", strlen ("Suerte con tu horario"));
			}
			
			g++;
			
			r = RANDOM(10) + 2;
			printf ("Tiempo: %i\n", r);
			inicio = fin;
		}
	}
	printf ("El cliente lento se cierra\n");
	close (c);
	
	return 0;
}

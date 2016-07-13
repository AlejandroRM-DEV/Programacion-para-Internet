/*
*   Alejandro Ramirez Muñoz
*   213496617
*   Programacion para Internet - 2016V
*   Practica SSL
*/
/*
    Para compilar
    g++ -std=c++11  *.cpp -lssl -lcrypto
*/
#include <sys/time.h>
#include <cstdio>
#include <iostream>
#include <sstream>

#include <openssl/ssl.h>
#include <openssl/x509.h>

#include "SocketPortable.h"
#include "URLParser.h"

using namespace std;

#define TAMANO_BUFFER 8192

long long milisegundos();
bool inputError();
URL* obtenerURL();
bool realizarPeticionHTTP(URL* url, FILE* temp);
bool realizarPeticionHTTPS(URL* url, FILE* temp);
void procesarRespuestaHTTP(FILE* temp);

/**
    Pruebas:
    HTTP:               : http://entropymag.org/wp-content/uploads/2014/10/outer-space-wallpaper-pictures.jpg
    HTTPS (no confiable): https://entropymag.org/wp-content/uploads/2014/10/outer-space-wallpaper-pictures.jpg
    HTTPS (confiable)   : https://upload.wikimedia.org/wikipedia/commons/d/da/Internet2.jpg
*/

int main() {
  FILE* temp;
  bool peticionCompleta;
  URL* url = obtenerURL();
  if (url == nullptr) {
    return -1;
  }
  // tempbuf facilita la separacion de la respuesta y los datos,
  // \r\n\r\n puede llegar separado y complica encontrarlo facilmente
  temp = fopen("tempbuf.dat", "w+b");
  if (temp == NULL) {
    cout << "Error al crear el archivo temporal \"tempbuf.dat\" necesario"
         << endl;
    return -1;
  }
  if (url->protocolo == "http" || url->protocolo == "HTTP") {
    peticionCompleta = realizarPeticionHTTP(url, temp);
  } else {
    peticionCompleta = realizarPeticionHTTPS(url, temp);
  }
  if (peticionCompleta) {
    procesarRespuestaHTTP(temp);
    fclose(temp);
    return 0;
  } else {
    fclose(temp);
    return -1;
  }
}

bool realizarPeticionHTTP(URL* url, FILE* temp) {
  long long milisegundosActuales;
  char buffer[TAMANO_BUFFER + 1];
  struct addrinfo hints;
  int totalBytes;
  SocketPortable sp;
  string peticion = "GET " + url->ruta + " HTTP/1.1" + "\r\nHost: " +
                    url->host + "\r\nConnection: close\r\n\r\n";

  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (!sp.connect(url->host.c_str(), url->puerto.c_str(), &hints) &&
      !sp.setNonBlock()) {
    cout << sp.getLastErrorMessage() << endl;
    return false;
  }

  cout << "Socket listo." << endl << "Enviando peticion. . ." << endl;
  totalBytes = sp.send(peticion.c_str(), peticion.length(), 0);
  if (totalBytes < 0) {
    perror("ERROR FATAL");
    return false;
  } else if (totalBytes == 0) {
    cout << "Conexion cerrada" << endl;
    return false;
  }

  cout << "Recibiendo respuesta. . ." << endl;
  totalBytes = -1;
  milisegundosActuales = milisegundos();
  while (totalBytes != 0) {
    if ((milisegundos() - milisegundosActuales) >= 50) {
      totalBytes = sp.recv((char*)&buffer, TAMANO_BUFFER, 0);
      if (totalBytes < 0) {
        if (!sp.nonBlockNoError()) {
          perror("ERROR FATAL");
          return false;
        }
      } else if (totalBytes > 0) {
        buffer[totalBytes] = 0;
        fwrite(buffer, sizeof(char), totalBytes, temp);
      }
      cout << ". " << std::flush;
      milisegundosActuales = milisegundos();
    }
  }
  cout << "Conexion cerrada" << endl;
  sp.close();
  fflush(temp);
  return true;
}

bool realizarPeticionHTTPS(URL* url, FILE* temp) {
  long long milisegundosActuales;
  char buffer[TAMANO_BUFFER + 1];
  int totalBytes, ret, i;
  size_t pos1, pos2;
  long verify_err;
  char continuar;
  struct addrinfo hints;
  string str, peticion;
  SocketPortable sp;
  X509_NAME* certificado_nombre = NULL;
  X509* certificado = NULL;
  SSL_CTX* ctx;
  SSL* ssl;

  OpenSSL_add_all_algorithms();

  if (SSL_library_init() < 0) {
    cout << "No se pudo iniciar la libreria OpenSSL" << endl;
    return false;
  }

  if ((ctx = SSL_CTX_new(SSLv23_client_method())) == NULL) {
    cout << "No se pudo crear el contexto SSL" << endl;
    return false;
  }

  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
  ssl = SSL_new(ctx);

  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (!sp.connect(url->host.c_str(), url->puerto.c_str(), &hints) &&
      !sp.setNonBlock()) {
    cout << sp.getLastErrorMessage() << endl;
    return false;
  }

  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
  if (SSL_CTX_load_verify_locations(ctx, NULL, "/etc/ssl/certs/") != 1) {
    cout << "No se cargaron los certificados seguros guardados" << endl;
    return false;
  }

  SSL_set_fd(ssl, sp.getFD());
  if (SSL_connect(ssl) != 1) {
    cout << "No se pudo crear la sesion SSL" << endl;
    return false;
  }
  cout << "Sesion SSL/TLS establecida" << endl;

  verify_err = SSL_get_verify_result(ssl);
  if (verify_err != X509_V_OK) {
    cout << "Advertencia la conexion no es confiable\r\nDescripcion: "
         << X509_verify_cert_error_string(verify_err) << endl;
    do {
      cout << "Desea continuar? (S/N): ";
      cin >> continuar;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } while (inputError() || (continuar != 'S' && continuar != 's' &&
                              continuar != 'N' && continuar != 'n'));
    if (continuar == 'N' || continuar == 'n') return false;
  }

  certificado = SSL_get_peer_certificate(ssl);
  if (certificado == NULL) {
    cout << "No se pudo obtener el certificado " << endl;
    return false;
  }

  certificado_nombre = X509_NAME_new();
  certificado_nombre = X509_get_subject_name(certificado);
  cout << "Datos del certificado: " << endl;

  /*** Metodo legado ***/
  str = X509_NAME_oneline(certificado_nombre, nullptr, 0);
  pos1 = str.find("/CN=") + 4;
  pos2 = str.find("/", pos1);
  cout << "Common Name ( CN ): " << str.substr(pos1, pos2 - pos1) << endl;

  cout << "Enviando peticion. . ." << endl;
  peticion = "GET " + url->ruta + " HTTP/1.1" + "\r\nHost: " + url->host + "\r\nConnection: close\r\n\r\n";
  totalBytes = SSL_write(ssl, peticion.c_str(), peticion.length());
  if (totalBytes < 0) {
    perror("ERROR FATAL");
    return false;
  } else if (totalBytes == 0) {
    cout << "Conexion cerrada" << endl;
    return false;
  }

  cout << "Recibiendo respuesta. . ." << endl;
  totalBytes = -1;
  milisegundosActuales = milisegundos();
  while (totalBytes != 0) {
    if ((milisegundos() - milisegundosActuales) >= 50) {
      totalBytes = SSL_read(ssl, (char*)&buffer, TAMANO_BUFFER);
      if (totalBytes < 0) {
        if (SSL_get_error(ssl, totalBytes) != SSL_ERROR_WANT_READ &&
            SSL_get_error(ssl, totalBytes) != SSL_ERROR_WANT_WRITE) {
          perror("ERROR FATAL");
          return false;
        }
      } else if (totalBytes > 0) {
        buffer[totalBytes] = 0;
        fwrite(buffer, sizeof(char), totalBytes, temp);
      }
      cout << ". " << std::flush;
      milisegundosActuales = milisegundos();
    }
  }
  cout << "Conexion cerrada" << endl;
  sp.close();
  fflush(temp);

  SSL_free(ssl);
  X509_free(certificado);
  SSL_CTX_free(ctx);
}

void procesarRespuestaHTTP(FILE* temp) {
  string respuesta, aux, codigo, descripcionCodigo, archivoNombre;
  size_t offsetTemp, offsetArchivo;
  char buffer[TAMANO_BUFFER];
  char guardar;
  FILE* archivo;
  bool finHead = false;

  fseek(temp, 0, SEEK_SET);
  do {
    fread(buffer, sizeof(char), TAMANO_BUFFER, temp);
    aux = buffer;
    offsetTemp = aux.find("\r\n\r\n");
    if (offsetTemp == string::npos) {
      respuesta += aux;
    } else {
      respuesta += aux.substr(0, offsetTemp);
      finHead = true;
    }
  } while (!finHead);

  fseek(temp, offsetTemp + 4, SEEK_SET);  // +4 ( \r\n\r\n )

  istringstream issRespuesta(respuesta);
  issRespuesta >> aux >> codigo >> descripcionCodigo;

  if (codigo == "200") {
    do {
      cout << "Desea guardar el archivo? (S/N): ";
      cin >> guardar;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } while (inputError() || (guardar != 'S' && guardar != 's' &&
                              guardar != 'N' && guardar != 'n'));
    if (guardar == 'S' || guardar == 's') {
      do {
        cout << "Ingresa el nombre del archivo: ";
        getline(cin, archivoNombre);
      } while (inputError() || archivoNombre.empty());

      cout << "Guardando. . . " << endl;
      archivo = fopen(archivoNombre.c_str(), "wb");
      if (archivo != NULL) {
        offsetArchivo = fread(buffer, sizeof(char), TAMANO_BUFFER, temp);
        while (offsetArchivo == TAMANO_BUFFER) {
          fwrite(buffer, sizeof(char), TAMANO_BUFFER, archivo);
          offsetArchivo = fread(buffer, sizeof(char), TAMANO_BUFFER, temp);
        }
        fwrite(buffer, sizeof(char), offsetArchivo, archivo);
        fclose(archivo);
      } else {
        cout << "No es posible guardar el archivo" << endl;
      }
    } else {
      cout << "Saliendo. . ." << endl;
    }
  } else {
    cout << "ERROR: " << codigo << " " << descripcionCodigo << endl;
  }
}

URL* obtenerURL() {
  string str, peticion;

  cout << "Dame la URL: ";
  getline(cin, str);

  URL* url = URLParser::parse(str);
  if (url == nullptr) {
    cout << "URL \"" << str << "\" MAL FORMADA" << endl;
    return nullptr;
  }
  if (url->protocolo.empty()) {
    url->protocolo = "http";
  } else if (url->protocolo != "http" && url->protocolo != "HTTP" &&
             url->protocolo != "https" && url->protocolo != "HTTPS") {
    cout << "Protocolo no soportado, saliendo..." << endl;
    return nullptr;
  }
  if (url->puerto.empty() &&
      (url->protocolo == "http" || url->protocolo == "HTTP")) {
    url->puerto = "80";
  } else if (url->puerto.empty() &&
             (url->protocolo == "https" || url->protocolo == "HTTPS")) {
    url->puerto = "443";
  }
  if (url->ruta.empty()) {
    url->ruta = "/";
  }
  return url;
}

long long milisegundos() {
  struct timeval te;
  gettimeofday(&te, nullptr);
  long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
  return milliseconds;
}

bool inputError() {
  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
  } else {
    return false;
  }
}

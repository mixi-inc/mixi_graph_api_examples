#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define BUF_LEN 256

int main(argc, argv)
     int argc;
     char* argv[];
{
  int s;
  int ret;

  struct hostent* servhost;
  struct sockaddr_in server;
  struct servent* service;

  SSL* ssl;
  SSL_CTX* ctx;

  char request[BUF_LEN];

  char* host = "secure.mixi-platform.com";
  char* path = "/2/token";

  servhost = gethostbyname(host);
  if (servhost == NULL) {
    fprintf(stderr, "Failed to change from [%s] to IP address.\n", host);
    exit(1);
  }

  // bzero((char*)&server, sizeof(server));
  memset((char*)&server, 0, sizeof(server));
  server.sin_family = AF_INET;

  // bcopy(servhost->h_addr, (char*)&server.sin_addr, servhost->h_length);
  memcpy((char*)&server.sin_addr, servhost->h_addr, servhost->h_length);

  service = getservbyname("https", "tcp");
  if (service != NULL) {
    server.sin_port = service->s_port;
  } else {
    server.sin_port = htons(443);
  }

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    fprintf(stderr, "Failed to create a socket.\n");
    exit(1);
  }

  if (connect(s, (struct sockaddr*)&server, sizeof(server)) == -1) {
    fprintf(stderr, "Failed to connect.\n");
    exit(1);
  }

  SSL_load_error_strings();
  SSL_library_init();
  ctx = SSL_CTX_new(SSLv23_client_method());
  if (ctx == NULL) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  ssl = SSL_new(ctx);
  if (ssl == NULL) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  ret = SSL_set_fd(ssl, s);
  if (ret == 0) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  RAND_poll();
  while(RAND_status() == 0) {
    unsigned short rand_ret = rand() % 65536;
    RAND_seed(&rand_ret, sizeof(rand_ret));
  }

  ret = SSL_connect(ssl);
  if (ret != 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  sprintf(request,
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n\r\n",
          path, host);
  ret = SSL_write(ssl, request, strlen(request));
  if (ret < 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  while(1) {
    char buf[BUF_LEN];
    int read_size;
    read_size = SSL_read(ssl, buf, sizeof(buf) - 1);

    if (read_size > 0) {
      buf[read_size] = '\0';
      write(1, buf, read_size);
    } else if (read_size == 0) {
      break;
    } else {
      ERR_print_errors_fp(stderr);
      exit(1);
    }
  }

  ret = SSL_shutdown(ssl);
  if (ret != 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }
  close(s);

  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
}

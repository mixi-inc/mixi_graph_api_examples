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

int get_http_status_code(response)
     const char* response;
{
  char buf[3];

  strncpy(buf, response + sizeof(char) * 9, 3);
  return atoi(buf);
}

char* new_http_response_body(response)
     const char* response;
{
  char* body;
  char* base_pos;
  char* temp_pos;
  char buf[BUF_LEN];
  int len;
  
  base_pos = strstr(response, "\r\n\r\n") + 4;
  temp_pos = strstr(base_pos, "\r\n");
  len = (int)(temp_pos - base_pos);
  strncpy(buf, base_pos, len);
  buf[len] = '\0';
  len = (int)strtol(buf, NULL, 16);

  body = (char*)malloc(sizeof(char) * len + 1);
  strncpy(body, temp_pos + 2, len);
  body[len + 1] = '\0';

  return body;
}

char* issue_token(client_id, client_secret, redirect_uri, authorization_code)
     char* client_id;
     char* client_secret;
     char* redirect_uri;
     char* authorization_code;
{
  char* host = "secure.mixi-platform.com";
  char* path = "/2/token";

  int s;
  int ret;
  int cnt;
  int sum;

  struct hostent* servhost;
  struct sockaddr_in server;

  SSL* ssl;
  SSL_CTX* ctx;

  char request[BUF_LEN];
  char* response;

  servhost = gethostbyname(host);
  if (servhost == NULL) {
    fprintf(stderr, "Failed to change from [%s] to IP address.\n", host);
    exit(1);
  }
  memset((char*)&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  memcpy((char*)&server.sin_addr, servhost->h_addr, servhost->h_length);
  server.sin_port = htons(443);
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
          "POST %s HTTP/1.1\r\n"
          "Host: %s\r\n\r\n",
          path, host);
  ret = SSL_write(ssl, request, strlen(request));
  if (ret < 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  response = (char*)malloc(BUF_LEN);
  cnt = 1;
  sum = 0;
  while(1) {
    char buf[BUF_LEN];
    int read_size;
    read_size = SSL_read(ssl, buf, sizeof(buf));
    if (read_size > 0) {
      if ((sum + read_size + 1) > (sizeof(char) * BUF_LEN * cnt)) {
	response = (char*)realloc(response,
				  sizeof(char) * BUF_LEN * ++cnt + 1);
      }
      memcpy(response + sum, buf, read_size);
      sum += read_size;
    } else if (read_size == 0) {
      response[sum + 1] = '\0';
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

  return response;
}

int main(argc, argv)
     int argc;
     char* argv[];
{
  char* token_response;
  int token_status_code;
  char* token_body;

  token_response = issue_token(NULL, NULL, NULL, NULL);
  token_status_code = get_http_status_code(token_response);
  token_body = new_http_response_body(token_response);
  if (token_status_code != 200) {
    printf("%d: %s\n", token_status_code, token_body);
  } else {
  }

  free(token_body);
  free(token_response);

  return 0;
}

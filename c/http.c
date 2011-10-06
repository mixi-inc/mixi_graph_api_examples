#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include "http.h"

#define BUF_LEN 256

char* new_json_property_value(char* json, char* property_name) {
  char* property;
  int len;
  char* base_pos;
  char* end_pos;
  char* result;

  len = strlen(property_name);
  property = malloc(len + 3);
  property[0] = '"';
  strncpy(property + 1, property_name, len);
  property[len + 1] = '"';
  property[len + 2] = '\0';

  base_pos = strstr(json, property);
  if (base_pos == NULL) {
    result = malloc(1);
    result[0] = '\0';
  } else {
    base_pos += strlen(property) + 2;
    end_pos = strstr(base_pos, "\"");
    len = (int)(end_pos - base_pos);
    
    result = malloc(len + 1);
    strncpy(result, base_pos, len);
    result[len] = '\0';
  }

  free(property);
  return result;
}

int get_http_status_code(const char* response) {
  char buf[4];

  strncpy(buf, response + 9, 3);
  buf[3] = '\0';
  return atoi(buf);
}

char* new_http_response_body(const char* response) {
  char* pos;
  char* result;

  pos = strstr(response, "\r\n\r\n") + 4;
  result = malloc(strlen(pos) + 1);
  strcpy(result, pos);

  return result;
}

int connect_to_server(char* host, int port, char* path) {
  int s;

  struct hostent* servhost;
  struct sockaddr_in server;

  servhost = gethostbyname(host);
  if (servhost == NULL) {
    fprintf(stderr, "Failed to change from [%s] to IP address.\n", host);
    exit(1);
  }
  memset((char*)&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  memcpy((char*)&server.sin_addr, servhost->h_addr, servhost->h_length);
  server.sin_port = htons(port);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    fprintf(stderr, "Failed to create a socket.\n");
    exit(1);
  }
  if (connect(s, (struct sockaddr*)&server, sizeof(server)) == -1) {
    fprintf(stderr, "Failed to connect.\n");
    exit(1);
  }

  return s;
}

struct ssl_info ssl_initialize(int s) {
  int ret;
  unsigned short rand_ret;

  SSL* ssl;
  SSL_CTX* ctx;

  struct ssl_info result;

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
    rand_ret = rand() % 65536;
    RAND_seed(&rand_ret, sizeof(rand_ret));
  }
  ret = SSL_connect(ssl);
  if (ret != 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  result.ssl = ssl;
  result.ctx = ctx;
  return result;
}

char* receive_http_response(enum ePROTOCOL protocol, int s, SSL* ssl) {
  int cnt;
  int sum;
  int read_size;

  char buf[BUF_LEN];
  char* response;

  response = malloc(BUF_LEN);
  cnt = 1;
  sum = 0;
  while(1) {
    if (protocol == HTTPS) {
      read_size = SSL_read(ssl, buf, sizeof(buf));
    } else {
      read_size = read(s, buf, sizeof(buf));
    }
    if (read_size > 0) {
      if ((sum + read_size + 1) > (BUF_LEN * cnt)) {
        response = realloc(response, BUF_LEN * ++cnt + 1);
      }
      memcpy(response + sum, buf, read_size);
      sum += read_size;
    } else if (read_size == 0) {
      response[sum] = '\0';
      break;
    } else {
      ERR_print_errors_fp(stderr);
      exit(1);
    }
  }

  return response;
}

void close_connection(enum ePROTOCOL protocol, int s, struct ssl_info* ssl) {
  int ret;

  if (protocol == HTTPS) {
    ret = SSL_shutdown(ssl->ssl);
    if (ret != 1) {
      ERR_print_errors_fp(stderr);
      exit(1);
    }
    SSL_free(ssl->ssl);
    SSL_CTX_free(ssl->ctx);
    ERR_free_strings();
  }
  close(s);
}

void send_http_request(enum ePROTOCOL protocol, char* request, int s, SSL* ssl) {
  int ret;

  if (protocol == HTTPS) {
    ret = SSL_write(ssl, request, strlen(request));
  } else {
    ret = write(s, request, strlen(request));
  }
  if (ret < 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }
}

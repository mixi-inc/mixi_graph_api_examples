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

struct ssl_info {
  SSL* ssl;
  SSL_CTX* ctx;
};

struct client_credential {
  char* client_id;
  char* client_secret;
  char* redirect_uri;
};

struct http_response {
  int status_code;
  char* body;
};

int get_string_byte_length(source)
     char* source;
{
  return strlen(source) * sizeof(char);
}

char* new_json_property_value(json, property_name)
     char* json;
     char* property_name;
{
  char* property;
  int len;
  char* base_pos;
  char* end_pos;
  char* result;

  len = strlen(property_name);
  property = (char*)malloc(sizeof(char) * (len + 3));
  property[0] = '"';
  strncpy(property + 1, property_name, len);
  property[len + 1] = '"';
  property[len + 2] = '\0';

  base_pos = strstr(json, property) + strlen(property) + 2;
  end_pos = strstr(base_pos, "\"");
  len = (int)(end_pos - base_pos);

  result = (char*)malloc(sizeof(char) * (len + 1));
  strncpy(result, base_pos, len);
  result[len] = '\0';

  free(property);
  return result;
}

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

  body = (char*)malloc(sizeof(char) * (len + 1));
  strncpy(body, temp_pos + 2, len);
  body[len] = '\0';

  return body;
}

int connect_to_server(host, port, path)
     char* host;
     int port;
     char* path;
{
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

struct ssl_info ssl_initialize(s)
     int s;
{
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

void send_issue_token_request(cc, authorization_code, host, path, ssl)
     struct client_credential cc;
     char* authorization_code;
     char* host;
     char* path;
     SSL* ssl;
{
  int ret;
  char request[2048];
  char post_body[2048];
  int len;
  

  sprintf(post_body,
          "grant_type=authorization_code&client_id=%s"
          "&client_secret=%s&redirect_uri=%s&code=%s",
          cc.client_id,
          cc.client_secret,
          cc.redirect_uri,
          authorization_code);
  sprintf(request,
          "POST %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Content-Type: application/x-www-form-urlencoded\r\n"
          "Content-Length: %d\r\n\r\n%s",
          path,
          host,
          (int)strlen(post_body) * sizeof(char),
          post_body);
  ret = SSL_write(ssl, request, strlen(request));
  if (ret < 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }
}

char* receive_issue_token_response(ssl)
     SSL* ssl;
{
  int cnt;
  int sum;
  int read_size;

  char buf[BUF_LEN];
  char* response;

  response = (char*)malloc(BUF_LEN);
  cnt = 1;
  sum = 0;
  while(1) {
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

  return response;
}

void close_connection(s, ssl)
     int s;
     struct ssl_info ssl;
{
  int ret;

  ret = SSL_shutdown(ssl.ssl);
  if (ret != 1) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }
  close(s);

  SSL_free(ssl.ssl);
  SSL_CTX_free(ssl.ctx);
  ERR_free_strings();
}

struct http_response issue_token(cc, authorization_code)
     struct client_credential cc;
     char* authorization_code;
{
  char* host;
  char* path;
  char* response;
  int ret;
  int s;
  struct ssl_info ssl;
  struct http_response result;

  host = "secure.mixi-platform.com";
  path = "/2/token";

  s = connect_to_server(host, 443, path);
  ssl = ssl_initialize(s);

  send_issue_token_request(cc, authorization_code, host, path, ssl.ssl);
  response = receive_issue_token_response(ssl.ssl);

  close_connection(s, ssl);

  result.status_code = get_http_status_code(response);
  result.body = new_http_response_body(response);

  free(response);
  return result;
}

int main(argc, argv)
     int argc;
     char* argv[];
{
  char* error;
  char* access_token;
  struct http_response token_response;

  if (argc != 2) {
    printf("Error: Authorization code is required.\n");
    exit(1);
  }

  struct client_credential cc = {
    "aa67b0abb047fcdde340",
    "",
    "http%3A%2F%2Fmixi.jp%2Fconnect_authorize_success.html"
  };

  token_response = issue_token(cc, argv[1]);
  if (token_response.status_code != 200) {
    error = new_json_property_value(token_response.body, "error");
    printf("Error: (%d) %s\n", token_response.status_code, error);
    free(error);
  } else {
    access_token = new_json_property_value(token_response.body, "access_token");
    
    free(access_token);
  }

  free(token_response.body);

  return 0;
}

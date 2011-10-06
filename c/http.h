#include <openssl/ssl.h>

struct ssl_info {
  SSL* ssl;
  SSL_CTX* ctx;
};

struct http_response {
  int status_code;
  char* body;
};

enum ePROTOCOL {
  HTTP,
  HTTPS
};

char* new_json_property_value(char* json, char* property_name);

int get_http_status_code(const char* response);

char* new_http_response_body(const char* response);

int connect_to_server(char* host, int port, char* path);

struct ssl_info ssl_initialize(int s);

char* receive_http_response(enum ePROTOCOL protocol, int s, SSL* ssl);

void close_connection(enum ePROTOCOL protocol, int s, struct ssl_info* ssl);

void send_http_request(enum ePROTOCOL protocol, char* request, int s, SSL* ssl);

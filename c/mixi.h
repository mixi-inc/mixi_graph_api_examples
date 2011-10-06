#include <openssl/ssl.h>

struct client_credential {
  char* client_id;
  char* client_secret;
  char* redirect_uri;
};

void send_issue_token_request(struct client_credential cc,
                              char* authorization_code,
                              char* host,
                              char* path,
                              SSL* ssl);

void send_get_my_profile_request(char* access_token,
                                 char* host,
                                 char* path,
                                 int s);

struct http_response issue_token(struct client_credential cc,
                                 char* authorization_code);

struct http_response get_my_profile(char* access_token);

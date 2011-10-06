#include "utils.h"
#include "http.h"
#include "mixi.h"

#define BUF_LEN 256

void send_issue_token_request(struct client_credential cc,
                              char* authorization_code,
                              char* host,
                              char* path,
                              SSL* ssl) {
  int ret;
  int len;
  int digit;
  char* post_body;
  char* request;
  char* post_body_len;

  post_body =
    new_concat_strings(8,
                       "grant_type=authorization_code&client_id=",
                       cc.client_id,
                       "&client_secret=",
                       cc.client_secret,
                       "&redirect_uri=",
                       cc.redirect_uri,
                       "&code=",
                       authorization_code);
  len = (int)(strlen(post_body) * sizeof(char));
  digit = get_integer_digit(len);
  post_body_len = (char*)malloc(sizeof(char) * 4);
  sprintf(post_body_len, "%d", len);
  request =
    new_concat_strings(11,
                       "POST ",
                       path,
                       " HTTP/1.1\r\n",
                       "Host: ",
                       host,
                       "\r\n",
                       "Content-Type: application/x-www-form-urlencoded\r\n",
                       "Content-Length: ",
                       post_body_len,
                       "\r\n\r\n",
                       post_body);
  send_http_request(HTTPS, request, 0, ssl);

  free(post_body);
  free(post_body_len);
  free(request);
}

void send_get_my_profile_request(char* access_token,
                                 char* host,
                                 char* path,
                                 int s) {
  char* request;
  int ret;

  request =
    new_concat_strings(10,
                       "GET ",
                       path,
                       " HTTP/1.1\r\n",
                       "Host: ",
                       host,
                       "\r\n",
                       "Accept: application/json\r\n",
                       "Authorization: OAuth ",
                       access_token,
                       "\r\n\r\n");
  send_http_request(HTTP, request, s, NULL);

  free(request);
}

struct http_response issue_token(struct client_credential cc,
                                 char* authorization_code) {
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
  response = receive_http_response(HTTPS, 0, ssl.ssl);

  close_connection(HTTPS, s, &ssl);

  result.status_code = get_http_status_code(response);
  result.body = new_http_response_body(response);

  free(response);
  return result;
}

struct http_response get_my_profile(char* access_token) {
  int s;
  char* host;
  char* path;
  char* response;
  struct http_response result;

  host = "api.mixi-platform.com";
  path = "/2/people/@me/@self";

  s = connect_to_server(host, 80, path);
  send_get_my_profile_request(access_token, host, path, s);
  response = receive_http_response(HTTP, s, NULL);

  close_connection(HTTP, s, NULL);

  result.status_code = get_http_status_code(response);

  result.body = new_http_response_body(response);

  free(response);
  return result;
}

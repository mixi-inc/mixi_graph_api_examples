#include "http.h"
#include "mixi.h"

int main(int argc, char* argv[]) {
  char* error;
  char* access_token;
  char* display_name;
  struct http_response token_response;
  struct http_response my_profile_response;

  if (argc != 2) {
    printf("Error: Authorization code is required.\n");
    exit(1);
  }

  struct client_credential cc = {
    "YOUR CLIENT ID",
    "YOUR CLIENT SECRET",
    "YOUR REDIRECT URI"
  };

  token_response = issue_token(cc, argv[1]);
  if (token_response.status_code != 200) {
    error = new_json_property_value(token_response.body, "error");
    printf("Error at retrieving token: (%d) %s\n",
	   token_response.status_code, error);

    free(error);
  } else {
    access_token =
      new_json_property_value(token_response.body, "access_token");
    my_profile_response = get_my_profile(access_token);
    if (my_profile_response.status_code != 200) {
      printf("Error at retrieve peron: (%d)\n",
	     my_profile_response.status_code);
    } else {
      display_name =
	new_json_property_value(my_profile_response.body, "displayName");
      printf("%s\n", display_name);
      free(display_name);
    }

    free(access_token);
    free(my_profile_response.body);
  }

  free(token_response.body);
  return 0;
}

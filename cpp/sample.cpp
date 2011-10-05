#include <iostream>
#include "clx/https.h"
#include "clx/uri.h"
#include "clx/ssl.h"
#include "clx/json.h"

using namespace std;

class mixi_client {
public:
    mixi_client() {
        CLIENT_ID= "YOUR Client ID";
        CLIENT_SECRET = "YOUR Client secret";
        REDIRECT_URI = "YOUR Redirect URI";
        secure_host = "secure.mixi-platform.com";
        api_host =  "api.mixi-platform.com";
        token_path = "/2/token/";
    }

    void get_auth_token(string auth_code) {
        map<string, string>header;
        header.insert(map<string, string>::value_type( "Content-Type","application/x-www-form-urlencoded" ) );

        clx::https httpClient(secure_host, 443);
        string query = get_auth_token_param(auth_code);
        httpClient.post(token_path, query, header);
        if (httpClient.code() != 200) {
            std::cout << "error" << std::endl << httpClient.body() << std::endl;
            return ;
        }
        clx::json response(httpClient.body());
        access_token = response["access_token"];
        refresh_token = response["refresh_token"];
    }

    clx::json get_people(string access_token, string user, string group, bool do_retry=true){
        map<string, string>header;
        string token;
        token.append("OAuth ").append(access_token);
        header.insert(map<string, string>::value_type("Authorization", token));
        header.insert(map<string, string>::value_type("Content-Type", "application/x-www-form-urlencoded"));
        string path = "/2/people/";
        path.append(user).append("/")
            .append(group).append("/");
        clx::http httpClient(api_host);
        httpClient.get(clx::uri::encode(path), header);
        if (httpClient.code() != 200) {
            if (is_token_expired(httpClient) && do_retry) {
                return get_people(excahge_token(refresh_token),
                        user, group, false);
            }
            std::cout << "error code:" << httpClient.code() << std::endl
                      << httpClient.body() << std::endl;
            std::exit(-1);
        }
        clx::json results(httpClient.body());
        clx::json entity(results["entry"]);
        return entity;
    }

    clx::json get_people(string user, string group){
        return get_people(access_token,user,group);
    }

 private:
    const char* CLIENT_ID;
    const char* CLIENT_SECRET;
    const char* REDIRECT_URI;
    const char* secure_host;
    const char* api_host;
    const char* token_path;
    string access_token;
    string refresh_token;

    string excahge_token(string refresh_token) {
        map<string, string>header;
        header.insert(map<string, string>::value_type("Content-Type", "application/x-www-form-urlencoded"));

        clx::https httpClient(secure_host, 443);
        string query = get_refresh_token_param(refresh_token);
        httpClient.post(token_path, query, header);
        clx::json response(httpClient.body());
        access_token = response["access_token"];
        refresh_token = response["refresh_token"];
        return access_token;
    }

    bool is_token_expired(clx::http client){
        if (client.code() == 401) {
            if (client.head()["WWW-Authenticate"].length()) {
                return true;
            }
        }
        return false;
    }

    string get_refresh_token_param(string refresh_token) {
        ostringstream buff;
        buff << "client_id=" << CLIENT_ID
             << "&client_secret=" << CLIENT_SECRET
             << "&refresh_token=" << refresh_token
             << "&grant_type=refresh_token";
        return buff.str();
    }

    string get_auth_token_param(string code) {
        ostringstream buff;
        buff << "client_id=" << CLIENT_ID
             << "&client_secret=" << CLIENT_SECRET
             << "&redirect_uri=" << url_encode(REDIRECT_URI)
             << "&code=" << code
             << "&grant_type=authorization_code";
        return buff.str();
    }

    string url_encode(const string& src) {
        ostringstream buff;
        buff << std::hex << std::setfill('0');
        typedef string::const_iterator Iterator;
        Iterator end = src.end();
        for (Iterator itr = src.begin(); itr != end; ++itr) {
            if((*itr >= 'a' && *itr <= 'z')
                    || (*itr >= 'A' && *itr <= 'Z')
                    || (*itr >= '0' && *itr <= '9')
                    || *itr == '-' || *itr == '.'
                    || *itr == '_' || *itr == '~') {
                buff << *itr;
            } else {
                buff << '%' << std::setw(2)
                    << static_cast<int>(static_cast<unsigned char>(*itr));
            }
        }
        return buff.str();
    }
};

int main(int argc, char* argv[]) {

    mixi_client client ;
    if (argc > 1) {
        string auth_code = argv[1];
        client.get_auth_token(auth_code);
    }

    clx::json entity =client.get_people("@me","@self");
    for (clx::json::iterator pos = entity.begin(); pos != entity.end(); ++pos) {
        std::cout << pos->first << " " << pos->second << std::endl;
    }
    return 0;
}

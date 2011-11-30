package main

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"net/http/httputil"

	"net/url"
	"strings"
	"text/template"
)

type friend struct {
	DisplayName  string
	ThumbnailUrl string
	ProfileUrl   string
}

type tokens struct {
	AccessToken  string
	RefreshToken string
}

const (
	AUTHORIZE_URL_BASE   = "https://mixi.jp/connect_authorize.pl?response_type=code&scope=r_profile&client_id="
	TOKENS_ENDPOINT      = "https://secure.mixi-platform.com/2/token"
	FRIEND_LIST_ENDPOINT = "http://api.mixi-platform.com/2/people/@me/@friends"
	CONFIG_FILENAME      = "config.json"
	TOKENS_FILENAME      = "tokens.txt"
	TEMPLATE             = `
<html>
  <head><title>Friend Lit</title></head>
  <body>
    <ul>
    {.repeated section Friends}
      <li>
        <img src="{ThumbnailUrl}" width="30" height="30" />
        <a href="{ProfileUrl}">{DisplayName}</a>
      </li>
    {.end}
    </ul>
  </body>
</html>
`
)

var config map[string]string
var currentTokens *tokens

func NewTokens(accessToken string, refreshToken string) *tokens {
	return &tokens{accessToken, refreshToken}
}

func RestoreTokens() *tokens {
	if bytes, err := ioutil.ReadFile(TOKENS_FILENAME); err == nil {
		pair := strings.Split(string(bytes), "\n")
		return &tokens{pair[0], pair[1]}
	}
	return nil
}

func (t *tokens) store() {
	ioutil.WriteFile(TOKENS_FILENAME, []byte(t.AccessToken+
		"\n"+t.RefreshToken), 0666)
}

func authorizeCode(authCode string) (err error) {
	return updateTokens(map[string][]string{
		"grant_type":    {"authorization_code"},
		"client_id":     {config["client_id"]},
		"client_secret": {config["client_secret"]},
		"code":          {authCode},
		"redirect_uri":  {"http://localhost:" + config["redirect_port"]},
	})
}

func refreshToken() (err error) {
	return updateTokens(map[string][]string{
		"grant_type":    {"refresh_token"},
		"client_id":     {config["client_id"]},
		"client_secret": {config["client_secret"]},
		"refresh_token": {currentTokens.RefreshToken},
	})
}

func updateTokens(params map[string][]string) (err error) {
	println("Call: " + TOKENS_ENDPOINT)
	response, _ := http.PostForm(TOKENS_ENDPOINT, params)
	b, _ := ioutil.ReadAll(response.Body)
	println(string(b))

	var responseJson map[string]interface{}
	json.Unmarshal(b, &responseJson)
	if errorMessage, ok := responseJson["error"]; ok {
		return errors.New(errorMessage.(string))
	} else {
		currentTokens = &tokens{responseJson["access_token"].(string), responseJson["refresh_token"].(string)}
		currentTokens.store()
		return nil
	}
	return errors.New("access_token is null")
}

func oauthGet(accessToken string, urlString string) (*http.Response, error) {
	url_, _ := url.Parse(urlString)
	conn, _ := net.Dial("tcp", url_.Host+":80")

	clientConn := httputil.NewClientConn(conn, nil)
	header := map[string][]string{"Authorization": {"OAuth " + accessToken}}
	request := http.Request{Method: "GET", URL: url_, Header: header}
	clientConn.Write(&request)
	return clientConn.Read(&request)
}

func getFriendList() (friends []friend, err error) {
	println("Call: " + FRIEND_LIST_ENDPOINT)
	response, _ := oauthGet(currentTokens.AccessToken, FRIEND_LIST_ENDPOINT)
	if response.StatusCode == 401 {
		if err = refreshToken(); err == nil {
			return getFriendList()
		}
		return nil, err
	}
	b, _ := ioutil.ReadAll(response.Body)
	println(string(b))

	var responseJson map[string]interface{}
	json.Unmarshal(b, &responseJson)
	entry := responseJson["entry"].([]interface{})
	totalResults := int(responseJson["totalResults"].(float64))
	itemsPerPage := int(responseJson["itemsPerPage"].(float64))
	var itemsCount int
	if itemsPerPage < totalResults {
		itemsCount = itemsPerPage
	} else {
		itemsCount = totalResults
	}
	friends = make([]friend, itemsCount)
	for i := 0; i < itemsCount; i++ {
		data := entry[i].(map[string]interface{})
		friends[i] = friend{
			DisplayName:  data["displayName"].(string),
			ThumbnailUrl: data["thumbnailUrl"].(string),
			ProfileUrl:   data["profileUrl"].(string),
		}
	}
	return friends, nil
}

func redirect(writer http.ResponseWriter, request *http.Request, redirectUrl string) {
	println("Redirect to: " + redirectUrl)
	http.Redirect(writer, request, redirectUrl, http.StatusFound)
}

func handleFriendList(writer http.ResponseWriter, request *http.Request) {
	if request.URL.Path == "/favicon.ico" {
		return
	}

	var (
		friends []friend
		err     error
	)

	authorizeUrl := AUTHORIZE_URL_BASE + config["client_id"]
	parts := strings.Split(request.URL.RawPath, "?code=")
	if 2 <= len(parts) {
		if err = authorizeCode(parts[1]); err != nil {
			redirect(writer, request, authorizeUrl)
		} else {
			redirect(writer, request, "/")
		}
		return
	} else if currentTokens == nil {
		redirect(writer, request, authorizeUrl)
		return
	}

	if friends, err = getFriendList(); err != nil {
		redirect(writer, request, authorizeUrl)
	} else {
		params := new(struct{ Friends []friend })
		params.Friends = friends
		tmpl, _ := template.New("mixi").Parse(TEMPLATE)
		tmpl.Execute(writer, params)
	}
}

func main() {
	var (
		bytes []byte
		err   error
	)

	if bytes, err = ioutil.ReadFile(CONFIG_FILENAME); err != nil {
		log.Fatal("ioutil.ReadFile:", err)
	}
	if err = json.Unmarshal(bytes, &config); err != nil {
		log.Fatal("json.Unmarshal:", err)
	}
	currentTokens = RestoreTokens()

	http.Handle("/", http.HandlerFunc(handleFriendList))

	addr := "localhost:" + config["redirect_port"]
	println("Please open http://" + addr + " on a web browser.")
	if err = http.ListenAndServe(addr, nil); err != nil {
		log.Fatal("http.ListenAndServe:", err)
	}
}

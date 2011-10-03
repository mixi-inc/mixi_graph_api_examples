(ns clj-mixi-graph-api.core
  (:require [com.twinql.clojure.http :as http])
  (:require [clj-json.core           :as json]))

(def consumer-info 
  {:key          "xxxxxxxxxxxxxxxxxxxx"
   :secret       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
   :redirect-uri "https://mixi.jp/connect_authorize_success.html"})

(def endpoint
  {:token "https://secure.mixi-platform.com/2/token"
   :api   "http://api.mixi-platform.com/2"})

(defstruct token-info :refresh-token :access-token :expires-in)
(defn mk-token
  "make token-info structure from json-parsed response of request or refresh token"
  [parsed-resp]
  (let [{:strs [refresh_token access_token expires_in]} parsed-resp]
    (struct token-info refresh_token access_token expires_in)))

(defstruct person :id :thumbnail-url :display-name :profile-url)
(defn mk-person
  "make person structure from people element"
  [data]
  (let [{:strs [id thumbnailUrl displayName profileUrl]} data]
    (struct person id thumbnailUrl displayName profileUrl)))

(defn post-to-token-with
  "post request to token end point with query"
  [query]
  (http/post (endpoint :token)
    :query (assoc query :client_id     (consumer-info :key)
                        :client_secret (consumer-info :secret))
    :as :string))

(defn request-access-token [code]
  (post-to-token-with
    {:grant_type    "authorization_code"
     :code          code
     :redirect_uri  (consumer-info :redirect-uri)}))

(defn refresh-access-token [token-info]
  (post-to-token-with
    {:grant_type    "refresh_token"
     :refresh_token token-info}))

(defn get-token
  "get token-info structure from response"
  [{content :content}]
  (mk-token (json/parse-string content)))

(defn read-people
  [{token-info :token
    user-id    :user-id
    group-id   :group-id
    res-format :format}]
  (http/get (str (endpoint :api) "/people/" user-id "/" group-id)
            :query {:format      res-format
                    :oauth_token (token-info :access-token)}
            :as :string))

(defn build-headers [headers]
  (reduce
    (fn [res header] (assoc res (keyword (first header)) (second header)))
    {}
    headers))

(defn get-oauth-error [{oauth-error :WWW-Authenticate}]
  (nth (re-find #"OAuth error='(.+)'" oauth-error) 1))

(defn has-oauth-error? [{oauth-error :WWW-Authenticate}]
  (not (nil? oauth-error)))

(defn expired? [resp]
  (let [built-headers (build-headers (:headers resp))]
    (and
      (has-oauth-error? built-headers)
      (= "expired_token" (get-oauth-error built-headers)))))

(defn get-my-friends
  "sample of accessing a protected resource"
  [token-info]
  (let [resp (read-people {:token    token-info
                           :user-id  "@me"
                           :group-id "@friends"
                           :format   "json"})]
    (if (expired? resp)
      (recur (get-token (refresh-access-token (:refresh-token token-info))))
      (map mk-person (get (json/parse-string (:content resp)) "entry")))))

(ns clj-mixi-graph-api.test.core
  (:use [clj-mixi-graph-api.core :as m])
  (:use [clojure.test]))

(deftest test-mk-token
  (let [token {"refresh_token" "hoge"
               "access_token"  "fuga"
               "expires_in"    "piyo"}]
    (is (= {:refresh-token "hoge"
            :access-token  "fuga"
            :expires-in    "piyo"}
           (m/mk-token token)))))

(deftest test-mk-person
  (let [person {"id"          "foo" "thumbnailUrl" "bar"
                "displayName" "baz" "profileUrl"   "qux"}]
    (is (= {:id           "foo" :thumbnail-url "bar"
            :display-name "baz" :profile-url   "qux"}
           (m/mk-person person)))))

(deftest test-get-token
  (let [resp {:content "{\"refresh_token\":\"hoge\",\"access_token\":\"fuga\",\"expires_in\":\"piyo\"}"}]
    (is (= {:refresh-token "hoge"
            :access-token  "fuga"
            :expires-in    "piyo"}
           (m/get-token resp)))))

(deftest test-build-header
  (let [headers {"foo" "bar"}]
    (is (= "bar" (:foo (build-headers headers))))))

(deftest test-get-oauth-error
  (let [has-error-header         {:WWW-Authenticate "OAuth error='error!'"}
        has-expired-error-header {:WWW-Authenticate "OAuth error='expired_token'"}]
    (is (= "error!"        (m/get-oauth-error has-error-header)))
    (is (= "expired_token" (m/get-oauth-error has-expired-error-header)))))

(deftest test-has-oauth-error?
  (let [no-error-header          {:foo "bar"}
        has-error-header         {:WWW-Authenticate "OAuth error='error!'"}
        has-expired-error-header {:WWW-Authenticate "OAuth error='expired_token'"}]
    (is (not (m/has-oauth-error? no-error-header)))
    (is (m/has-oauth-error? has-error-header))
    (is (m/has-oauth-error? has-expired-error-header))))

(deftest test-expired?
  (let [no-error-resp          {:headers {"foo" "bar"}}
        has-error-resp         {:headers {"WWW-Authenticate" "OAuth error='error!'"}}
        has-expired-error-resp {:headers {"WWW-Authenticate" "OAuth error='expired_token'"}}]
    (is (not (m/expired? no-error-resp)))
    (is (not (m/expired? has-error-resp)))
    (is (m/expired? has-expired-error-resp))))

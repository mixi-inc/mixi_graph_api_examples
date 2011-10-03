# clj-mixi-graph-api

Clojure sample for using mixi Graph API.

## Usage

0. Leiningen を入れる

https://github.com/technomancy/leiningen 


1. ConsumeKey と ConsumerSecret 及び、必要があれば RedirectUri を書き換える

    (def consumer-info
      {:key          "xxxxxxxxxxxxxxxxxxxx"
       :secret       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       :redirect-uri "https://mixi.jp/connect_authorize_success.html"})

2. 依存を解決する

    % lein deps

2. REPLで実行する

request-access-token にはユーザーの認可で得た authorization code を渡してください。

    % lein repl
    clj-mixi-graph-api.core=> (def first-token (get-token (request-access-token "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")))
    clj-mixi-graph-api.core=> (get-my-friends first-token)
    clj-mixi-graph-api.core=> (def refreshed-token (get-token (refresh-access-token (:refresh-token first-token))))
    clj-mixi-graph-api.core=> (get-my-friends refreshed-token)

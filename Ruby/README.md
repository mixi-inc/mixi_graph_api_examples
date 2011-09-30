# 実行手順
## gemのインストール
JSONをインストールする。

    $gem install json

## コードの書き換え
コード内のconsumer_key,　consumer_secret,　redirect_uriを適切に書き換えてください。
## Authorization codeの入手
以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=[YOUR CONSUMER KEY]&scope=r_profile

## 実行

    $ ruby sample.rb
    input Authorization code > [codeパラメータ値]

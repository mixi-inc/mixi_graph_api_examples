# 実行方法

## スクリプト中の CONSUMER_KEY(Consumer Key), CONSUMER_SECRET(Consumer Secret), REDIRECT_URI(リダイレクトURI) を環境に応じて書き換えます

    CONSUMER_KEY='****'
    CONSUMER_SECRET='****'
    REDIRECT_URI='****'

## Authorization code の入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手します。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## 実行

access_token の入手

    $ ./mixi.sh auth <Authorization code>

access_token のリフレッシュ

    $ ./mixi.sh refresh <refresh_token>

PeopleAPI にて自分自身の情報を取得

    $ ./mixi.sh people self <access_token>

PeopleAPI にて友人の情報を取得

    $ ./mixi.sh people friends <access_token>

結果はそれぞれ JSON にて出力されます。
json_xs コマンドが利用可能な場合、JSON を整形して出力します。


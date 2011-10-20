# 実行手順
## コードの書き換え
コード内のCONSUMER_KEY,　CONSUMER_SECRET,　REDIRECT_URIを適切に書き換えてください。
## Authorization codeの入手
以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=[YOUR CONSUMER KEY]&scope=r_profile

## 実行

    $ sample.pl [codeパラメータ値]


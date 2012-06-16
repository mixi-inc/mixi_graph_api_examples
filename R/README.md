# 実行手順
## R packageのインストール
rjson,RCurlをインストールします。

    In your R console:
    > install.packages("rjson")
    > install.packages("RCurl")

RCurlの依存ライブラリとしてlibcurl([http://curl.haxx.se.]())が必要です。


## コードの書き換え
コード内のconsumer\_key,　consumer\_secret,　redirect\_uriを適切に書き換えてください。
## Authorization codeの入手
以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=[YOUR CONSUMER KEY]&scope=r_profile

## 実行

    $ Rscript sample.R
    Input Authorization Code : [codeパラメータ値]

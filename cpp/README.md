# 実行方法
## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## コンパイル時に必要なもの
    GCC(gcc (GCC) 4.1.2 20080704 で動作確認をしています)
    OpenSSL(http://www.openssl.org/)
    CLX(http://clx.cielquis.net/)

## 実行
    1. Makefileを必要に応じて変更する
    2. makeを実行する
    3. mixi_sample 取得したAuthorization Code を実行

# 実行手順

## 必要なもの

> OpenSSL

> GNU開発環境

## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## コードの修正

test.c ファイルにある以下の文字列を適切なものに変更する。

> YOUR CLIENT ID

> YOUR CLIENT SECRET

> YOUR REDIRECT URI

※ REDIRECT URIは、URLエンコード済みの文字列を記載してください。

## 実行

    $> make
    $> ./test codeパラメータ値

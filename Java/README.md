# 実行方法

## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## 実行

    $> cd Mixi.javaを保存したディレクトリパス 
    $> javac Mixi.java
    $> java Mixi codeパラメータ値

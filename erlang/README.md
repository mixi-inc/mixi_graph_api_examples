# 実行手順

##　mochiwebのインストール

    $> cd
    $> git clone git://github.com/mochi/mochiweb.git
    $> vi .bashrc
    $> source .bashrc

.bashrcには以下を記述する。

    export ERL_LIBS=githubから入手したmochiwebのディレクトリパス

## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## 実行

    $> cd mixi.erlとtest.erlを保存したディレクトリパス
    $> erl
    1> c(mixi).
    2> c(test).
    3> test:main("codeパラメータ値").

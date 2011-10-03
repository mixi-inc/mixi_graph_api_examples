# 実行方法

1. sbt 0.11 のセットアップ

https://github.com/harrah/xsbt/wiki/Setup を参考に sbt をセットアップする。

2. Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile
    
3. Consumer Key および Consumer Sercret を書き換える

    object Main {
        val CONSUMER_KEY    = "XXXXXXXX"
        val CONSUMER_SECRET = "XXXXXXXXXXXXXXXX"
         :
    }

4. 実行

    $> sbt
    > run <Authorization code>

example01.Main と example02.Main のどちらを実行するか聞かれるので、実行する方を選択します。
example01.Main ではユーザIDとグループIDの入力待ちになります。それぞれをスペース区切りで入力します。
example02.Main は友人一覧を取得して終了します。


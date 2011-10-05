# 実行手順

## 依存モジュールのインストール

> $ cabal install json

> $ cabal install http-enumerator

## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile

## Consumer Key, Consumer Secret を書きかえ

> $ vim show_friend.hs # clientId, clientSecret をそれぞれ適切な値に編集

## 実行

> $ runghc show_friend.hs ${取得したcode値}

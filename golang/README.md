# 実行手順

1. mixi Developer DashboardでリダイレクトURLを [http://localhost:8080] にしたアプリを登録（ポート番号は任意）
2. config.jsonを作成

    {
        "client_id":"YOUR CLIENT ID",
        "client_secret":"YOUR SECRET",
        "redirect_port":"8080"
    }

3. コンパイルしてサーバー起動

    $ 6g mixi.go
    $ 6l mixi.6
    $ 6.out
    Please open http://localhost:8080 on a web browser.

4. ブラウザで [http://localhost:8080] を開く

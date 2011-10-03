# 実行方法

## 準備

任意のエクセルのSheet1に本コードを貼り付け、フォームにボタンを３つ追加する。

    Btn_GetAccessToken （Authorization codeでAccess tokenを取得する）
    Btn_GetProfile （People APIでプロフィール情報を取得する）
    Btn_Refresh （Refresh tokenでAccess tokenを再発行する）

## Authorization codeの入手

以下のURLにWebブラウザにてアクセスし、リダイレクト先に含まれるcodeパラメータ値を入手する。

    https://mixi.jp/connect_authorize.pl?client_id=Consumer key&scope=r_profile
    
## 以下のセル番号に対応するセルにそれぞれ値を入力する

    Private Const CL_CLIENT_ID = "C5"          ' Consumer key
    Private Const CL_CLIENT_SECRET = "C6"      ' Consumer Secret
    Private Const CL_REDIRECT_URI = "C7"       ' Redirect URI
    Private Const CL_AUTHORIZATION_CODE = "C8" ' Authorization Code

## 実行

フォーム上のボタンを押下して、処理を実行する。

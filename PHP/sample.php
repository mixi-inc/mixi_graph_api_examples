<?php
define('CONSUMER_KEY', '[YOUR CONSUMER KEY]');
define('CONSUMER_SECRET', '[YOUR CONSUMER SECRET]');
define('REDIRECT_URI', '[YOUR REDIRECT URI]');

define('MIXI_TOKEN_ENDPOINT', 'https://secure.mixi-platform.com/2/token');
define('MIXI_API_ENDPOINT', 'http://api.mixi-platform.com/2');

function getToken($auth_code) {
    $data = array(
         "grant_type"    => "authorization_code",
         "client_id"     =>CONSUMER_KEY,
         "client_secret" => CONSUMER_SECRET,
         "code"          => $auth_code,
         "redirect_uri"  => REDIRECT_URI,
    );
    $data = http_build_query($data, "", "&");
    $context = array(
         "http" => array(
         "method" => "POST",
         "header" => implode("\r\n", array(
            "Content-Type: application/x-www-form-urlencoded",
            "Content-Length: ".strlen($data)
    )),
         "content" => $data
    )
    );
    $res =  file_get_contents(MIXI_TOKEN_ENDPOINT, false, stream_context_create($context));
    return json_decode($res,true);
}

function getNewToken($refreshToken){

    $data = array(
        "grant_type"    => "refresh_token",
        "client_id"     => CONSUMER_KEY,
        "client_secret" => CONSUMER_SECRET,
        "refresh_token" => $refreshToken,
    );
    $data = http_build_query($data, "", "&");
    $context = array(
        "http" => array(
            "method" => "POST",
            "header" => implode("\r\n", array(
                "Content-Type: application/x-www-form-urlencoded",
                "Content-Length: ".strlen($data)
    )),
        "content" => $data
    )
    );
    $json_res =  file_get_contents(MIXI_TOKEN_ENDPOINT, false, stream_context_create($context));
    $res = json_decode($json_res,true);
    return $res;
}

function request($endPoint,$token){
    $request = MIXI_API_ENDPOINT.$endPoint.'?&oauth_token='.$token['access_token'];
    $responseHeader = get_headers($request,1);
    if(array_key_exists('WWW-Authenticate',$responseHeader)){
        $errorMessage = $responseHeader['WWW-Authenticate'];
        if(preg_match('/invaild_request/',$errorMessage)){
            exit('Invaild Request');
        }elseif(preg_match('/invaild_token/',$errorMessage)){
            exit('Invaild Token');
        }elseif(preg_match('/expired_token/',$errorMessage)){
            $token = getNewToken($token['refresh_token']);
            return request($endPoint, $token);
        }elseif (preg_match('/insufficient_scope/',$errorMessage)) {
            exit('Insufficient Scope');
        }
    }
    $res = file_get_contents($request,TRUE);
    return json_decode($res,TRUE);
}
if ($argc != 2){
    exit("Please input your Authorization Code\nUsage : sample.php [YOUR AUTHORIZATION CODE]\n");
}
$auth_code = $argv[1];
$token = getToken($auth_code);
$json_array = request('/people/@me/@self', $token);
print_r($json_array);

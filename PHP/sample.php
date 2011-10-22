<?php
define('CONSUMER_KEY', '[YOUR CONSUMER KEY]');
define('CONSUMER_SECRET', '[YOUR CONSUMER SECRET]');
define('REDIRECT_URI', '[YOUR REDIRECT URI]');

class MixiGraphAPIExample
{
    const MIXI_API_ENDPOINT   = 'http://api.mixi-platform.com/2';
    const MIXI_TOKEN_ENDPOINT = 'https://secure.mixi-platform.com/2/token';

    private $token;

    public function __construct($auth_code) {
        $this->authorize($auth_code);
    }

    private function post($uri, $data) {
        $context = array('http' => array (
            'method' => 'POST',
            'header' => 'Content-Type: application/x-www-form-urlencoded',
            'content' => http_build_query($data, null, '&'),
            'ignore_errors' => true,
        ));
        $body = file_get_contents($uri, false, stream_context_create($context));
        $header = $this->parseHeader($http_response_header);
        if ($this->isHttpFail($header['Status'])) {
            throw new UnexpectedValueException('Post Request Fail:'.PHP_EOL.$uri.PHP_EOL.var_export($header, true));
        }
        return $body;
    }

    private function authorize($auth_code) {
        $data = array(
             'grant_type'    => 'authorization_code',
             'client_id'     => CONSUMER_KEY,
             'client_secret' => CONSUMER_SECRET,
             'code'          => $auth_code,
             'redirect_uri'  => REDIRECT_URI,
        );
        $this->token = json_decode($this->post(self::MIXI_TOKEN_ENDPOINT, $data), true);
    }

    private function refreshToken() {
        $data = array(
            'grant_type'    => 'refresh_token',
            'client_id'     => CONSUMER_KEY,
            'client_secret' => CONSUMER_SECRET,
            'refresh_token' => $this->token['refresh_token'],
        );
        $this->token = json_decode($this->post(self::MIXI_TOKEN_ENDPOINT, $data), true);
    }

    private function parseHeader($headers) {
        $statusLine = array_shift($headers);
        list(, $result['Status'], )  = explode(' ', $statusLine);
        foreach ($headers as $header) {
            list($key, $value) = explode(': ', $header);
            $result[$key] = $value;
        }
        return $result;
    }

    private function isHttpFail($status) {
        return (bool)(empty($status) || ($status >= 400));
    }

    private function isExpired($headers) {
        $result = false;
        if (array_key_exists('WWW-Authenticate', $headers)) {
            if (preg_match('/expired_token/', $headers['WWW-Authenticate'])) {
                $result = true;
            }
        }
        return $result;
    }

    private function call($location) {
        static $retry_count = 0;

        $uri = self::MIXI_API_ENDPOINT . $location . '?oauth_token=' . $this->token['access_token'];
        $response = file_get_contents($uri, false, stream_context_create(array('http' => array('ignore_errors' => true))));
        $header = $this->parseHeader($http_response_header);

        if ($this->isHttpFail($header['Status'])) {
            if ($this->isExpired($header)) {
                if ($retry_count++ > 1) {
                    throw new RangeException('Token Refresh Too many retry. '.PHP_EOL.var_export($this->token, true).PHP_EOL.var_export($header, true));
                }

                $this->refreshToken();
                $response = $this->call($location);
                $retry_count = 0;
            } else {
                throw new UnexpectedValueException('Invalid API Access:'.PHP_EOL.$uri.PHP_EOL.var_export($header, true));
            }
        }

        return $response;
    }

    public function execute($endpoint) {
        return json_decode($this->call($endpoint), true);
    }

    public static function getInstance($auth_code) {
        return new self($auth_code);
    }
}

if (debug_backtrace()) return;
if ($_SERVER['argc'] != 2) {
    exit("Please input your Authorization Code\n  Usage : {$_SERVER['argv'][0]} [YOUR AUTHORIZATION CODE]\n");
}

try {
    $response = \MixiGraphAPIExample::getInstance($_SERVER['argv'][1])->execute('/people/@me/@self');
    var_dump($response);
} catch (Exception $e) {
    var_dump($e->getMessage());
}

require 'rubygems'
require 'json'
require 'net/https'
require 'net/http'

$consumer_key = "[YOUR CONSUMER KEY]"
$consumer_secret = "[YOUR CONSUMER SECRET]"
$redirect_uri = "[YOUR REDIRECT URI]"


def getToken(auth_code)
  params = "grant_type=authorization_code&client_id=#{$consumer_key}&client_secret=#{$consumer_secret}&code=#{auth_code}&redirect_uri=#{$redirect_uri}"
  https = Net::HTTP.new('secure.mixi-platform.com',443)
  https.use_ssl = true
  https.start{
   response = https.post('/2/token', params).body
   return JSON.parse(response)
  }
end

def getNewToken(refreshToken)
  params = "grant_type=refresh_token&client_id=#{$consumer_key}&client_secret=#{$consumer_secret}&refresh_token=#{refreshToken}"
  https = Net::HTTP.new('secure.mixi-platform.com', 443)
  https.use_ssl = true
  result = nil
  https.start{
    response = https.post('/2/token',params).body
    result = JSON.parse(response)
  }
  return result
end

def request(endPoint,token)
  http = Net::HTTP.new('api.mixi-platform.com',80)
  result = nil
  http.start{
    response = http.get("/2#{endPoint}",
                        {'Authorization' => "OAuth #{token['access_token']}",
                        'HOST' => 'api.mixi-platform.com'}).body
    result = JSON.parse(response)
  }
  return result
end

print "input Authorization code > "
auth_code = STDIN.gets
auth_code.chomp!
token = getToken(auth_code)
puts 'Access Token : '<<token['access_token']
puts 'Refresh Token : '<<token['refresh_token']
puts request('/people/@me/@self',token)
token = getNewToken(token['refresh_token'])
puts 'Access Token : '<<token['access_token']
puts 'Refresh Token : '<<token['refresh_token']
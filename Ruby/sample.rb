require 'rubygems'
require 'json'
require 'net/https'
require 'net/http'
require 'pp'

module Mixi
  CONSUMER_KEY = '1dfbd65727d594eee136'
  CONSUMER_SECRET = '4193262f0aaa9f8af607da455efd25983ebacb1e'
  REDIRECT_URI = 'http://localhost:8008'

  class Client
    def self.example
      raise 'authcode is mandatory.' if ARGV.empty?
      client = Mixi::Client.new
      client.authorize! ARGV.first
      pp client.call('/people/@me/@self')
    end

    def authorize!(auth_code)
      params = "grant_type=authorization_code&client_id=#{CONSUMER_KEY}&client_secret=#{
        CONSUMER_SECRET}&code=#{auth_code}&redirect_uri=#{REDIRECT_URI}"
      https = Net::HTTP.new('secure.mixi-platform.com', 443)
      https.use_ssl = true
      https.start do
        response = https.post('/2/token', params).body
        json = JSON.parse(response)
        @access_token = json['access_token']
        @refresh_token = json['refresh_token']
      end
    end

    def call(endpoint)
      http = Net::HTTP.new('api.mixi-platform.com', 80)
      http.start do
        response = http.get("/2#{endpoint}",
                            'Authorization' => "OAuth #{@access_token}",
                            'HOST' => 'api.mixi-platform.com').body
        JSON.parse(response)
      end
    end
  end
end

Mixi::Client.example

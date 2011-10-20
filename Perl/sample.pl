#!/usr/bin/perl

use strict;
use warnings;

use constant {
    CONSUMER_KEY    => '[YOUR CONSUMER KEY]',
    CONSUMER_SECRET => '[YOUR CONSUMER SECRET]',
    REDIRECT_URI    => '[YOUR REDIRECT URI]',

    MIXI_TOKEN_ENDPOINT => 'https://secure.mixi-platform.com/2/token',
    MIXI_API_ENDPOINT   => 'http://api.mixi-platform.com/2'
};

use URI::Escape qw/ uri_escape /;
use HTTP::Request;
use LWP::UserAgent;
use JSON::XS;
use Data::Dumper;

sub request {
    my ($method, $url, $data_arr) = @_;

    my $req = HTTP::Request->new(
        $method => $url
    );

    if ($method eq 'POST') {
        my $data_str = join '&', map { uri_escape($_) . '=' . uri_escape($data_arr->{$_}) } keys %$data_arr;
        $req->content_type('application/x-www-form-urlencoded');
        $req->content($data_str);
    }

    my $ua = LWP::UserAgent->new();
    my $res = $ua->request($req);

    return $res;
}

sub get_token {
    my $auth_code = shift;

    my %data_arr = (
        'grant_type'    => 'authorization_code',
        'client_id'     => CONSUMER_KEY,
        'client_secret' => CONSUMER_SECRET,
        'code'          => $auth_code,
        'redirect_uri'  => REDIRECT_URI,
    );

    my $res = request('POST', MIXI_TOKEN_ENDPOINT, \%data_arr);
    die 'Request failed. ' . $res->status_line unless $res->is_success;

    return decode_json($res->content);
}

sub get_new_token {
    my $refresh_token = shift;

    my %data_arr = (
        'grant_type'    => 'refresh_token',
        'client_id'     => CONSUMER_KEY,
        'client_secret' => CONSUMER_SECRET,
        'refresh_token' => $refresh_token,
    );

    my $res = request('POST', MIXI_TOKEN_ENDPOINT, \%data_arr);
    die 'Request failed. ' . $res->status_line unless $res->is_success;

    return decode_json($res->content);
}

sub call {
    my ($endpoint, $token) = @_;

    my $url = MIXI_API_ENDPOINT . $endpoint . '?oauth_token=' . uri_escape($token->{'access_token'});
    my $res = request('GET', $url);

    if (defined $res->header('WWW-Authenticate')) {
        my $error_msg = $res->header('WWW-Authenticate');

        if ($error_msg =~ /invalid_request/) {
            die 'Invalid request.';
        } elsif ($error_msg =~ /invalid_token/) {
            die 'Invalid token.';
        } elsif ($error_msg =~ /expired_token/) {
            $token = get_new_token($token->{'refresh_token'});
            $url = MIXI_API_ENDPOINT . $endpoint . '?oauth_token=' . uri_escape($token->{'access_token'});
            return decode_json(request('GET', $url)->{'_content'});
        } elsif ($error_msg =~ /insufficient_scope/) {
            die 'Insufficient scope.';
        }
    }

    return decode_json($res->content);
}

my $auth_code = $ARGV[0];
my $token = get_token($auth_code);
my $json_href = call('/people/@me/@self', $token);

print Data::Dumper::Dumper($json_href);

1;


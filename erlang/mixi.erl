-module(mixi).
-export([get_token/4,
	 get_my_friends/1]).

get_token(AuthorizationCode,
         ClientId,
         ClientSecret,
         RedirectUri) ->
    Response = httpc:request(
                 post,
                 {
                   "https://secure.mixi-platform.com/2/token",
                   [],
                   "application/x-www-form-urlencoded",
                   lists:append([
                                 "grant_type=authorization_code",
                                 "&client_id=", ClientId,
                                 "&client_secret=", ClientSecret,
                                 "&redirect_uri=", edoc_lib:escape_uri(RedirectUri),
                                 "&code=", AuthorizationCode
                                ])
                 },
                 [],
                 []),
    parse_response(Response,
                   fun(Json) ->
                           { get_property_value("access_token", Json),
                             get_property_value("refresh_token", Json) }
                   end
                  ).

get_my_friends(AccessToken) ->
    Response = httpc:request(
                 get,
                 {
                   "http://api.mixi-platform.com/2/people/@me/@friends",
                   [
                    {"Authorization", lists:append("OAuth ", AccessToken)}
                   ]
                 },
                 [],
                 []),
    parse_response(Response,
                   fun(Json) ->
                           Json
                   end
                  ).

parse_response({ok, {{_, 200, _}, _, Body}}, F) ->
    {struct, Json} = mochijson2:decode(Body),
    F(Json);
parse_response({ok, {{_, Status, _}, Headers, Body}}, _) ->
    case lists:keyfind("www-authenticate", 1, Headers) of
        {_, Reason} ->
            throw(
              { Status,
                Reason }
             );
        _ ->
            {struct, Error} = mochijson2:decode(Body),
            throw(
              { Status,
                get_property_value("error", Error),
                get_property_value("error_description", Error) }
             )
    end.

get_property_value(KeyName, Json) ->
    case lists:keyfind(list_to_binary(KeyName), 1, Json) of
        {_, Value} ->
            binary_to_list(Value);
        _ ->
            []
    end.

-module(test).
-export([main/1]).

main(AuthorizationCode) ->
    initialize(),
    ClientId = "[Client ID]",
    ClientSecret = "[Client secret]",
    RedirectUri = "[Redirect URI]",
    try mixi:get_token(AuthorizationCode,
                  ClientId,
                  ClientSecret,
                  RedirectUri) of
        {AccessToken, RefreshToken} ->
	    io:format("Access token: ~p Refresh token: ~p~n",
		      [AccessToken, RefreshToken]),
            Response = mixi:get_my_friends(AccessToken),
	    {_, Entries} = lists:keyfind(<<"entry">>, 1, Response),
	    lists:foreach(fun({struct, Entry}) ->
				  {_, DisplayName} = lists:keyfind(
						       <<"displayName">>, 1, Entry),
				  io:format("~ts~n", [DisplayName])
			  end, Entries)
    catch
        throw:{Status, Error, Description} ->
            io:format("~p: ~s : ~s~n",
                      [Status, Error, Description])
    end,
    cleanup().

initialize() ->
    inets:start(),
    ssl:start().

cleanup() ->
    ssl:stop(),
    inets:stop().

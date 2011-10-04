{-# LANGUAGE OverloadedStrings #-}
import Text.JSON
import System.Environment
import Network.HTTP.Enumerator
import Data.List (sort)
import Data.ByteString.Char8 hiding (sort, putStrLn)
import qualified Data.ByteString.Lazy.Char8 as L

data Token = Token {
  accessToken  :: String,
  refreshToken :: String
  } deriving Show

clientId     = "B" :: ByteString
clientSecret = "xyzzy" :: ByteString
redirectUri  = "https://mixi.jp/connect_authorize_success.html" :: ByteString

tokenRequest :: Monad m => String -> Request m
tokenRequest code =
  urlEncodedBody [("grant_type", "authorization_code"),
                  ("client_id", clientId),
                  ("client_secret", clientSecret),
                  ("code", pack code),
                  ("redirect_uri", redirectUri)]
  $ def { host = "secure.mixi-platform.com",
          port = 443,
          path = "/2/token",
          secure = True }

peopleRequest :: Monad m => String -> Request m
peopleRequest token = def {
  host = "api.mixi-platform.com",
  path = "/2/people/@me/@friends",
  requestHeaders = [
    ("Authorization", append "OAuth " (pack token))
    ]
  }

parseTokenResponse :: String -> Result Token
parseTokenResponse res = do
  json <- decode res
  case sort $ fromJSObject json of
    [("access_token",  JSString acc),
     ("expires_in", _),
     ("refresh_token", JSString ref),
     ("scope", _)] ->
      return $ Token {
        accessToken  = fromJSString acc,
        refreshToken = fromJSString ref
        }
    _ -> fail ("couldn't handle response:" ++ res)

getAccessToken :: String -> IO (Result Token)
getAccessToken code = do
  res <- withManager (httpLbs $ tokenRequest code)
  return $ parseTokenResponse (L.unpack $ responseBody res)

showFriends :: Token -> IO ()
showFriends (Token token _) =
  withManager (httpLbs $ peopleRequest token) >>= L.putStrLn . responseBody

main :: IO ()
main = do code:_ <- getArgs
          token  <- getAccessToken code
          case token of
            Ok token  -> showFriends token
            Error err -> putStrLn err

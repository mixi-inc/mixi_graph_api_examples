#demonstration

#load library
source("./lib/mixiGraphAPI.R")
library(rjson)
library(RCurl)

grant_type     ="authorization_code"
client_id      ="YOUR CONSUMER KEY"
client_secret  ="YOUR CONSUMER SECRET"
redirect_uri   ="YOUR REDIRECT URI"

cat("Input Authorization Code :  ")
code           <-readLines(file("stdin"),1)

token<-get_token(client_id,client_secret,code,redirect_uri)
print(token)

people<-get_people("@friends",token$access_token)
print(people)

refreshed_token<-refresh_token(token$refresh_token,client_id,client_secret)
print(refreshed_token)


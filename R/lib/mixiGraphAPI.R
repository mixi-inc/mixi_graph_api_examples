get_token<-function(client_id,client_secret,code,redirect_uri){
    token_raw<-postForm("https://secure.mixi-platform.com/2/token",
                            grant_type=grant_type,
                            client_id=client_id,
                            client_secret=client_secret,
                            code=code,
                            redirect_uri=redirect_uri
                        )
    parser<-newJSONParser()
    parser$addData(token_raw)
    token <- parser$getObject()
    return(token)
}


get_people<-function(type,access_token){
    url<-paste("https://api.mixi-platform.com/2/people/@me/",type,sep="")
    res_people_raw<-getForm(url,oauth_token=access_token)
    parser<-newJSONParser()
    parser$addData(res_people_raw)
    res_people <- parser$getObject()
    return(res_people)
}


refresh_token<-function(refresh_token,client_id,client_secret){
    res_raw_token <- postForm("https://secure.mixi-platform.com/2/token",
                                grant_type="refresh_token",
                                client_id=client_id,
                                client_secret=client_secret,
                                refresh_token=refresh_token
                             )
    parser<-newJSONParser()
    parser$addData(res_raw_token)
    res_token=parser$getObject()
    return(res_token)
}


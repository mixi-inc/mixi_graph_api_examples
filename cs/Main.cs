using System;
using System.Text;
using System.Net;
using System.Web.Script.Serialization;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace MixiGraphAPIExample
{
    class MainClass
    {
        private const string CONSUMER_KEY    = "[YOUR CONSUMER KEY]";
        private const string CONSUMER_SECRET = "[YOUR CONSUMER SECRET]";
        private const string REDIRECT_URL    = "[YOUR REDIRECT URL]";
        
        public static void Main (string[] args)
        {
            MixiAPIClient client = new MixiAPIClient(CONSUMER_KEY,CONSUMER_SECRET,REDIRECT_URL);
            
            Console.WriteLine("1. Open this url and allow your application access.");
            Console.WriteLine(client.GetAuthorizeUrl());
            
            Console.WriteLine("2. Input 'code' parameter value of redirected url.");
            string code = Console.ReadLine();
            client.GetToken(code);
            
            Console.WriteLine("----");
            string resText = client.Call("/people/@me/@self");
            Dictionary<string,object> result = MixiAPIClient.JsonToDictionary(resText);
            Dictionary<string,object> entry = (Dictionary<string, object>)result["entry"];
            foreach(string key in entry.Keys){
                Console.WriteLine(key + " : " + entry[key]);
            }
        }
    }
    
    class MixiAPIClient
    {
        private const string MIXI_TOKEN_ENDPOINT = "https://secure.mixi-platform.com/2/token";
        private const string MIXI_API_ENDPOINT   = "http://api.mixi-platform.com/2";
        private const string MIXI_AUTHORIZE_URL  = "https://mixi.jp/connect_authorize.pl";  
        
        private string consumer_key;
        private string consumer_secret;
        private string redirect_url;
        
        public string token = "";
        public string refreshToken = "";
        
        public static Dictionary<string,object> JsonToDictionary(string jsonText)
        {
            JavaScriptSerializer serializer = new JavaScriptSerializer();
            return  serializer.Deserialize<Dictionary<string,object>>(jsonText);
        }
        
        public MixiAPIClient(string consumer_key, string consumer_secret, string redirect_url)
        {
            this.consumer_key = consumer_key;
            this.consumer_secret = consumer_secret;
            this.redirect_url = redirect_url;
        }
        
        public string GetAuthorizeUrl()
        {
            return MIXI_AUTHORIZE_URL + "?scope=r_profile&client_id=" + consumer_key;
        }       
        
        public void GetToken(string authorizationCode)
        {
            NameValueCollection data = new NameValueCollection();
            data.Add ("grant_type","authorization_code");
            data.Add ("client_id",consumer_key);
            data.Add ("client_secret",consumer_secret);
            data.Add ("redirect_uri",redirect_url);
            data.Add ("code",authorizationCode);
            WebClient wc = new WebClient();
            Byte[] resData = wc.UploadValues(MIXI_TOKEN_ENDPOINT, data);
            string resText = Encoding.UTF8.GetString(resData);
            Dictionary<string,object> result = MixiAPIClient.JsonToDictionary(resText);
            token = result["access_token"].ToString();
            refreshToken = result["refresh_token"].ToString();
        }
        
        public string Call(string endpoint)
        {
            string url = MIXI_API_ENDPOINT + endpoint + "?oauth_token=" + token;
            WebClient wc = new WebClient();
            byte[] resData = wc.DownloadData(url);
            return Encoding.UTF8.GetString(resData);
        }   
    }
}


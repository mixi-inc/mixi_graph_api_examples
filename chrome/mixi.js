var CLIENT_ID = "1dfbd65727d594eee136";
var CLIENT_SECRET = "4193262f0aaa9f8af607da455efd25983ebacb1e";
var REDIRECT_URI = encodeURIComponent("http://localhost:8008");

function authorize(code) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function(){
    if (xhr.readyState == 4 && xhr.status == 200){
      var json = eval('(' + xhr.responseText + ')');
      document.getElementById('access_token').innerHTML = json['access_token'];
    }
  }
  xhr.open('POST', 'https://secure.mixi-platform.com/2/token');
  xhr.setRequestHeader("Content-Type" , "application/x-www-form-urlencoded");
  xhr.send("grant_type=authorization_code&client_id=" + CLIENT_ID + 
    "&client_secret=" + CLIENT_SECRET + "&code=" + code + "&redirect_uri=" + REDIRECT_URI);
}

function call(accessToken) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function(){
    if (xhr.readyState == 4 && xhr.status == 200){
      document.getElementById('json').value = xhr.responseText;
    }
  }
  xhr.open('GET', 'http://api.mixi-platform.com/2/people/@me/@friends');
  xhr.setRequestHeader("Authorization" , "OAuth " + accessToken);
  xhr.send();
}

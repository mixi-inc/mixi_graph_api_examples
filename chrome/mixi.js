var CLIENT_ID = "YOUR CLIENT ID";
var CLIENT_SECRET = "YOUR CLIENT SECRET";
var REDIRECT_URI = encodeURIComponent("YOUR REDIRECT URI");

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

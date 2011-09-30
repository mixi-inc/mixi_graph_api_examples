import java.net.URL;
import java.net.HttpURLConnection;
import java.net.URLEncoder;
import java.io.PrintWriter;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

public class Mixi {

    private static final String CLIENT_ID = "YOUR Client ID";
    private static final String CLIENT_SECRET = "YOUR Client secret";
    private static final String REDIRECT_URI = "YOUR Redirect URI";

    private String accessToken;

    public void authorize(String authorizationCode) throws IOException {
        PrintWriter out = null;
        try {
            String params = buildParamsForAuthorization(authorizationCode);
            URL url = new URL("https://secure.mixi-platform.com/2/token");
            HttpURLConnection connection = (HttpURLConnection)url.openConnection();
            connection.setDoOutput(true);
            out = new PrintWriter(new OutputStreamWriter(
                    connection.getOutputStream(), "UTF-8"));
            out.print(params);
            out.flush();
            int responseCode = connection.getResponseCode();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                String responseBody = readStream(connection.getInputStream());
                accessToken = getPropertyValue(responseBody, "access_token");
            } else {
                String responseBody = readStream(connection.getErrorStream());
                throw new IOException(responseCode + ": " + responseBody);
            }
        } finally {
            if (out != null) {
                out.close();
            }
        }
    }

    public String call(String endpoint) throws IOException {
        URL url = new URL("http://api.mixi-platform.com/2" + endpoint);
        HttpURLConnection connection = (HttpURLConnection)url.openConnection();
        connection.setRequestProperty("Authorization", "OAuth " + accessToken);
        int responseCode = connection.getResponseCode();
        if (responseCode == HttpURLConnection.HTTP_OK) {
            String responseBody = readStream(connection.getInputStream());
            return responseBody;
        } else {
            String responseBody = readStream(connection.getErrorStream());
            throw new IOException(responseCode + ": " + responseBody);
        }        
    }

    private String buildParamsForAuthorization(String authorizationCode) {
        try {
            StringBuilder sb = new StringBuilder();
            sb.append("client_id=");
            sb.append(CLIENT_ID);
            sb.append("&client_secret=");
            sb.append(CLIENT_SECRET);
            sb.append("&redirect_uri=");
            sb.append(URLEncoder.encode(REDIRECT_URI, "UTF-8"));
            sb.append("&code=");
            sb.append(authorizationCode);
            sb.append("&grant_type=authorization_code");
            return sb.toString();
        } catch(UnsupportedEncodingException e) {
            throw new IllegalStateException(e);
        }
    }

    private String readStream(InputStream in) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        try {
            byte[] buf = new byte[1024];
            int len = -1;
            BufferedInputStream bis = new BufferedInputStream(in, 1024);
            while((len = bis.read(buf, 0, 1024)) != -1) {
                out.write(buf);
            }
            return new String(out.toByteArray(), "UTF-8");
        } finally {
            try {
                out.close();
            } catch(IOException e) {
                throw new IllegalStateException(e);
            }
        }
    }

    private String getPropertyValue(String json, String propertyName) {
        String name = "\"" + propertyName + "\":\"";
        int pos = json.indexOf(name) + name.length();
        String value = json.substring(pos, json.indexOf("\"", pos + 1));
        return value;
    }

    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            throw new IllegalArgumentException("Authorization code is mandatory.");
        }
        String authorizationCode = args[0];
        Mixi mixi = new Mixi();
        mixi.authorize(authorizationCode);
        String response = mixi.call("/people/@me/@self");
        System.out.println(mixi.getPropertyValue(response, "displayName"));
    }

}
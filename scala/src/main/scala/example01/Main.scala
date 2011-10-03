package example01

import scala.annotation.tailrec

import dispatch.json.{Js, JsArray, JsObject, JsString, JsValue}
import org.apache.commons.httpclient.{HttpClient, HttpMethodBase, HttpStatus}
import org.apache.commons.httpclient.methods.{GetMethod, PostMethod}
import sjson.json.{DefaultProtocol, Format, JsonSerialization, Reads}

case class Tokens(refreshToken: String, expiresIn: Int, accessToken: String)
case class Response(entries: List[Entry])
case class Entry(id: String, displayName: String, thumbnailUrl: String, profileUrl: String)

object Protocols extends DefaultProtocol {
  implicit val TokensFormat: Format[Tokens] =
    asProduct3("refresh_token", "expires_in", "access_token")(Tokens)(Tokens.unapply(_).get)
  implicit val EntryFormat: Format[Entry] =
    asProduct4("id", "displayName", "thumbnailUrl", "profileUrl")(Entry)(Entry.unapply(_).get)

  implicit object ResponseFormat extends Reads[Response] {
    import JsonSerialization.fromjson
    def reads(js: JsValue) = js match {
      case JsObject(x) => x(JsString("entry")) match {
        case y: JsObject => Response(fromjson[Entry](y) :: Nil)
        case ys: JsArray => Response(fromjson[List[Entry]](ys))
        case y => sys.error("Require object or array.(%s)".format(y))
      }
      case x => sys.error("Require object.(%s)".format(x))
    }
  }
}

object Main {
  import JsonSerialization.fromjson
  import Protocols._

  val CONSUMER_KEY    = "<Your consumer key>"
  val CONSUMER_SECRET = "<Your consumer secret>"

  val TOKEN_ENDPOINT    = "https://secure.mixi-platform.com/2/token"
  val API_ENDPOINT_BASE = "http://api.mixi-platform.com/2"
  val REDIRECT_URI      = "http://mixi.jp/connect_authorize_success.html"

  val REQUEST_TOKEN_BASE_PARAMS = Map("client_id"     -> CONSUMER_KEY,
                                      "client_secret" -> CONSUMER_SECRET)

  def main(args: Array[String]) {
    args match {
      case Array(authorizationCode, _*) => repl(getTokens(authorizationCode))
      case _ => printf("""Usage: sbt run <Authorization Code>
                         |  please access the following url and get Autohrization Code.
                         |  https://mixi.jp/connect_authorize.pl?client_id=%s&response_type=code&scope=r_profile
                         |""".stripMargin,
                       CONSUMER_KEY)
    }
  }

  def getTokens(authorizationCode: String): Tokens =
    retrieveTokens(REQUEST_TOKEN_BASE_PARAMS +
                   ("grant_type"   -> "authorization_code") +
                   ("code"         -> authorizationCode) +
                   ("redirect_uri" -> REDIRECT_URI))

  def refreshTokens(tokens: Tokens): Tokens =
    retrieveTokens(REQUEST_TOKEN_BASE_PARAMS +
                   ("grant_type"    -> "refresh_token") +
                   ("refresh_token" -> tokens.refreshToken))

  def retrieveTokens(params: Map[String, String]): Tokens =
    using(new PostMethod(TOKEN_ENDPOINT)) { method =>
      for ((name, value) <- params)
        method.addParameter(name, value)

      new HttpClient executeMethod(method) match {
        case HttpStatus.SC_OK =>
          fromjson[Tokens](Js(method.getResponseBodyAsStream))
        case _ =>
          sys.error("Failure: %s".format(method.getStatusLine))
      }
    }

  val InputFormat = """\A\s*(@me|\w+)\s+(@self|@friends|\w+)\s*\z""".r
  @tailrec
  def repl(tokens: Tokens) {

    def read = Console.readLine("Input <User-ID> <Group-ID>: ") match {
      case InputFormat(userId, groupId) => Some((userId, groupId))
      case _ => None
    }

    def eval(userId: String, groupId: String): (Tokens, List[Entry]) =
      request(tokens, "/people/%s/%s".format(userId, groupId)) match {
        case (nextTokens, Some(value)) => (nextTokens,
                                           fromjson[Response](value).entries)
        case (nextTokens, None) => (nextTokens, Nil)
      }

    def print(entries: List[Entry]) =
      for {
        Entry(id, displayName, thumbnailUrl, profileUrl) <- entries
        text = """Entry(id: %s
                 |      displayName: %s
                 |      thumbnailUrl: %s
                 |      profileUrl: %s)""".stripMargin
               .format(id, displayName, thumbnailUrl, profileUrl)
      } println(text)

    val nextTokens = for {
      (userId, groupId) <- read
      (tokens, entries) = eval(userId, groupId)
    } yield {
      print(entries)
      tokens
    }

    repl(nextTokens.getOrElse(tokens))
  }

  val ErrorPattern = """\A\s*OAuth\s+.*error='([^']+)'.*\z""".r
  @tailrec
  def request(tokens: Tokens, path: String): (Tokens, Option[JsValue]) = {

    def _request(accessToken: String, path: String): Either[Any, JsValue] =
      using(new GetMethod(API_ENDPOINT_BASE + path)) { method =>
        method.setDoAuthentication(false)
        method.addRequestHeader("Authorization",
                                "OAuth %s".format(accessToken))
        new HttpClient executeMethod(method) match {
          case HttpStatus.SC_OK => Right(Js(method.getResponseBodyAsStream))
          case HttpStatus.SC_UNAUTHORIZED =>
            method.getResponseHeaders("WWW-Authenticate").map(_.getValue) match {
              case Array(ErrorPattern(reason), _*) => Left(Symbol(reason))
              case _ => Left(method.getStatusLine)
            }
          case _ => Left(method.getStatusLine)
        }
      }

    _request(tokens.accessToken, path) match {
      case Right(js) => (tokens, Some(js))
      case Left('expired_token) => request(refreshTokens(tokens), path)
      case Left(reason) => printf("Failure.(reason: %s)\n", reason)
                           (tokens, None)
    }
  }

  def using[A <: HttpMethodBase, B](method: A)(block: A => B): B =
    try { block(method) }
    finally { method.releaseConnection }
}

package example02

import scala.annotation.tailrec
import dispatch.json.{Js, JsObject, JsString, JsValue}
import org.apache.commons.httpclient.{HttpClient, HttpMethodBase, HttpStatus}
import org.apache.commons.httpclient.methods.{GetMethod, PostMethod}

trait Response
case class Ok(body: java.io.InputStream) extends Response
case class Unauthorized(headers: Map[String, String]) extends Response

object Conversions {
  implicit def jsObject2Map(js: JsObject): Map[JsString, JsValue] = js.self
  implicit def jsString2String(js: JsString): String = js.self
  implicit def string2JsString(value: String): JsString = JsString(value)

  implicit def any2Caster[T](value: T): Caster[T] = new Caster[T](value)
  class Caster[T](value: T) {
    def as[U<:T:Manifest]: Option[U] =
      if (manifest[U].erasure.isInstance(value)) Some[U](value.asInstanceOf[U])
      else None
  }
}

object Main extends App {
  import Conversions._

  val CONSUMER_KEY    = "3c5b3c9653f2afef6f58"
  val CONSUMER_SECRET = "48de82bbe76854ba1da84c21185c4c4ddc1bd22b"
  val REDIRECT_URI    = "http://mixi.jp/connect_authorize_success.html"
  val REQUEST_TOKEN_BASE_PARAMS = Map("client_id"     -> CONSUMER_KEY,
                                      "client_secret" -> CONSUMER_SECRET)

  for {
    authorizationCode <- args.headOption
    tokens            <- getTokens(authorizationCode)
    friends           <- get(tokens, "/people/@me/@friends")
  } println(friends)

  def getTokens(authorizationCode: String): Option[(String, String)] =
    retrieveTokens(REQUEST_TOKEN_BASE_PARAMS +
                   ("grant_type"   -> "authorization_code") +
                   ("code"         -> authorizationCode) +
                   ("redirect_uri" -> REDIRECT_URI))

  def refreshTokens(refreshToken: String): Option[(String, String)] =
    retrieveTokens(REQUEST_TOKEN_BASE_PARAMS +
                   ("grant_type"    -> "refresh_token") +
                   ("refresh_token" -> refreshToken))

  def retrieveTokens(params: Map[String, String]): Option[(String, String)] =
    using(new PostMethod("https://secure.mixi-platform.com/2/token")) { method =>
      for ((name, value) <- params) method.addParameter(name, value)
      for {
	response     <- executeHttpMethod(method)
        Ok(body)     <- response.as[Ok]
        x            <- Js(body).as[JsObject]
        refreshToken <- x.get("refresh_token").flatMap(_.as[JsString])
        accessToken  <- x.get("access_token").flatMap(_.as[JsString])
      } yield (refreshToken, accessToken)
    }

  val OAuthError = """\s*OAuth\s+error='([^']+)'""".r
  @tailrec
  def get(tokens: (String, String), path: String): Option[JsValue] = {
    def _get(accessToken: String, url: String): Either[Symbol, JsValue] =
      using(new GetMethod(url)) { method =>
	method.setDoAuthentication(false)
        method.addRequestHeader("Authorization", "OAuth " + accessToken)
	executeHttpMethod(method) match {
	  case Some(Ok(body)) => Right(Js(body))
	  case Some(Unauthorized(headers)) => Left(
	    headers.get("WWW-Authenticate") match {
	      case Some(OAuthError(reason)) => Symbol(reason)
	      case _ => 'unauthorized
	    })
	  case _ => Left('other)
	}
      }

    _get(tokens._2, "http://api.mixi-platform.com/2" + path) match {
      case Right(result) => Some(result)
      case Left('expired_token) => refreshTokens(tokens._1) match {
	case Some(nextTokens) => get(nextTokens, path)
	case _ => None
      }
      case _ => None
    }
  }

  def executeHttpMethod(method: HttpMethodBase): Option[Response] =
    new HttpClient executeMethod(method) match {
      case HttpStatus.SC_OK => Some(Ok(method.getResponseBodyAsStream))
      case HttpStatus.SC_UNAUTHORIZED => Some(Unauthorized(
	method.getResponseHeaders
	      .map { x => x.getName -> x.getValue } toMap
      ))
      case _ => None
    }

  def using[A <: HttpMethodBase, B](method: A)(block: A => B): B =
    try { block(method) }
    finally { method.releaseConnection }
}

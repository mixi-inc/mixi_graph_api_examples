#!/usr/bin/env python
# -*- coding: utf-8 -*-

from google.appengine.ext import webapp
from google.appengine.ext.webapp import util
import os
from google.appengine.ext.webapp import template
from google.appengine.ext.webapp.util import run_wsgi_app
from google.appengine.api import urlfetch
import urllib
from django.utils import simplejson

AUTHORIZATION_URL_BASE    = "https://mixi.jp/connect_authorize.pl?"
REDIRECTION_URI           = "http://localhost:8080/redirect"
TOKEN_ENDPOINT            = "https://secure.mixi-platform.com/2/token"
FRIENDS_TIMELINE_ENDPOINT = "http://api.mixi-platform.com/2/people/@me/@friends"

CONSUMER_KEY    = "*******************"
CONSUMER_SECRET = "**************************************"

class MainHandler(webapp.RequestHandler):

    def get_access_token(self,auth_code):

        token_params = {
                "grant_type":"authorization_code",
                "client_id":CONSUMER_KEY,
                "client_secret":CONSUMER_SECRET,
                "code":auth_code,
                "redirect_uri":REDIRECTION_URI,
                }
        form_data = urllib.urlencode(token_params)
        result = urlfetch.fetch(url=TOKEN_ENDPOINT,
                        payload=form_data,
                        method=urlfetch.POST,
                        headers={'Content-Type':'application/x-www-form-urlencoded'})

        json = simplejson.loads(result.content)
        return json['access_token']

    def get_friends_timeline(self,access_token):
        result = urlfetch.fetch(url=FRIENDS_TIMELINE_ENDPOINT,
                        method=urlfetch.GET,
                        headers={'Authorization':'OAuth %s'%access_token})
        return simplejson.loads(result.content)

    def get(self, mode=""):

        if mode == "login":

            query = {
                    "client_id":CONSUMER_KEY,
                    "response_type":"code",
                    "scope":"r_profile",
                    }
            url = AUTHORIZATION_URL_BASE + urllib.urlencode(query)
            self.redirect(url)

        if mode == "redirect":

            auth_code = self.request.get("code")
            access_token = self.get_access_token(auth_code)

            results = self.get_friends_timeline(access_token)

            template_values = {'results': results['entry'],}
            path = os.path.join(os.path.dirname(__file__), 'mixi.html')
            self.response.out.write(template.render(path, template_values))

def main():
    application = webapp.WSGIApplication([('/(.*)', MainHandler)],
                                         debug=True)
    util.run_wsgi_app(application)

if __name__ == '__main__':
    main()
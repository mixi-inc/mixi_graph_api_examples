#!/bin/bash

CONSUMER_KEY='****'
CONSUMER_SECRET='****'
REDIRECT_URI='****'

echo_json() {
    local json=$1

    local cmd_json_xs=`which json_xs 2>/dev/null`
    if [ $cmd_json_xs ]; then
        echo "$json" | $cmd_json_xs 2>/dev/null
        local retval=$?
        if [ $retval -eq 0 ]; then
            return
        fi
    fi

    echo "$json"
}

auth() {
    local code=$1
    local result=`curl -d grant_type=authorization_code \
                       -d client_id=$CONSUMER_KEY \
                       -d client_secret=$CONSUMER_SECRET \
                       -d code=$code \
                       -d redirect_uri=$REDIRECT_URI \
                       https://secure.mixi-platform.com/2/token \
                         2>/dev/null`
    local retval=$?
    if [ $retval -ne 0 ]; then
        echo "curl error" >&2
        return 1
    fi

    echo_json "$result"
    return 0
}

refresh() {
    local token=$1
    local result=`curl -d grant_type=refresh_token \
                       -d client_id=$CONSUMER_KEY \
                       -d client_secret=$CONSUMER_SECRET \
                       -d refresh_token=$token \
                       https://secure.mixi-platform.com/2/token \
                         2>/dev/null`
    local retval=$?
    if [ $retval -ne 0 ]; then
        echo "curl error" >&2
        return 1
    fi

    echo_json "$result"
    return 0
}

people() {
    local type=$1
    local token=$2
    local result=`curl -w "\n%{http_code}" \
                       https://secure.mixi-platform.com/2/people/@me/@$type?oauth_token=$token \
                         2>/dev/null`
    local retval=$?
    if [ $retval -ne 0 ]; then
        echo "curl error" >&2
        return 1
    fi

    local http_code=`echo "$result" | tail -n 1`
    if [ $http_code -ne 200 ]; then
        echo "ERROR (HTTP CODE = $http_code)" >&2
        return 1
    fi

    local line_count=`echo "$result" | wc -l`
    line_count=`expr $line_count - 1`
    local content=`echo "$result" | head -n $line_count`

    echo_json "$content"

    return 0
}

usage() {
    echo "Usage: $0 {auth|refresh|people} OPTIONS"
}

usage_auth() {
    echo "Usage: $0 auth CODE"
}

usage_refresh() {
    echo "Usage: $0 refresh REFRESH_TOKEN"
}

usage_people() {
    echo "Usage: $0 people {self|friends} ACCESS_TOKEN"
}

retval=1

case "$1" in
    auth | refresh)
       if [ $# -ne 2 ]; then
           usage_$1
           retval=1
       else
           $1 $2
           retval=$?
       fi
       ;;
    people)
       if [ $# -ne 3 ]; then
          usage_people
       else
          case "$2" in
              self | friends)
                  people $2 $3
                  ;;
              *)
                  usage_people
                  ;;
          esac
       fi
       ;;
    *)
       usage
       retval=1
       ;;
esac

exit $retval


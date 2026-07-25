#!/bin/bash

HOST0=http://localhost:8080
HOST1=http://localhost:8081
HOST2=http://localhost:8082

GREEN="\033[32m"
RED="\033[31m"
BLUE="\033[34m"
NC="\033[0m"

PASS=0
FAIL=0

pass() {
    echo -e "${GREEN}[PASS]${NC} $1"
    PASS=$((PASS+1))
}

fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    FAIL=$((FAIL+1))
}

section() {
    echo
    echo -e "${BLUE}========== $1 ==========${NC}"
}

expect_status() {

    local name="$1"
    local url="$2"
    local expected="$3"

    status=$(curl -s -o /dev/null -w "%{http_code}" "$url")

    if [ "$status" = "$expected" ]; then
        pass "$name"
    else
        fail "$name (expected $expected got $status)"
    fi
}

expect_body() {

    local name="$1"
    local url="$2"
    local text="$3"

    body=$(curl -s "$url")

    if echo "$body" | grep -q "$text"; then
        pass "$name"
    else
        fail "$name"
    fi
}

####################################################

section "SERVERS"

expect_body "Default server" "$HOST0/" "DEFAULT SERVER"
expect_body "Second server" "$HOST1/" "SITE 1"
expect_body "Third server" "$HOST2/" "SITE 2"

####################################################

section "STATIC"

expect_status "/" "$HOST0/" 200
expect_status "/images/" "$HOST0/images/" 200
expect_body "/images/" "$HOST0/images/" "IMAGES INDEX"

####################################################

section "AUTOINDEX"

expect_status "/autoindex/" "$HOST0/autoindex/" 200

####################################################

section "ERRORS"

expect_status "404" "$HOST0/noexiste" 404

####################################################

section "REDIRECT"

status=$(curl -s -o /dev/null -I -w "%{http_code}" "$HOST0/redirect")

if [ "$status" = "301" ]; then
    pass "Redirect"
else
    fail "Redirect"
fi

####################################################

section "METHODS"

status=$(curl -s -o /dev/null -X PUT -w "%{http_code}" "$HOST0/")

if [ "$status" = "405" ]; then
    pass "PUT forbidden"
else
    fail "PUT forbidden"
fi

####################################################

section "CGI"

expect_status "Python CGI" "$HOST0/cgi/cgi-bin/test.py" 200
expect_status "Bash CGI" "$HOST0/bash/cgi-bin/test.sh" 200

####################################################

section "UPLOAD"

status=$(curl \
-s \
-o /dev/null \
-w "%{http_code}" \
-X POST \
--data "hola" \
"$HOST0/uploads/")

echo "POST upload -> $status"

####################################################

echo
echo "==============================="
echo "Passed : $PASS"
echo "Failed : $FAIL"
echo "==============================="
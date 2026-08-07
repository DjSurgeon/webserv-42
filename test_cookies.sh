#!/bin/bash
./webserv conf/raul.conf &
SERVER_PID=$!
sleep 1

echo -e "\n--- 1. Login WITHOUT Cookie ---"
curl -s -i -X POST -d "username=Sergio_Admin" http://localhost:8080/api/login

echo -e "\n\n--- 2. Profile WITHOUT Cookie ---"
curl -s -i http://localhost:8080/api/profile

echo -e "\n\n--- 3. Login and Save Cookie ---"
curl -s -i -X POST -d "username=ValgrindMaster" -c cookie.txt http://localhost:8080/api/login

echo -e "\n\n--- 4. Profile WITH Cookie ---"
curl -s -i -b cookie.txt http://localhost:8080/api/profile

echo -e "\n\n--- 5. Logout ---"
curl -s -i -X POST -b cookie.txt -c cookie.txt http://localhost:8080/api/logout

echo -e "\n\n--- 6. Profile AFTER Logout ---"
curl -s -i -b cookie.txt http://localhost:8080/api/profile

kill $SERVER_PID

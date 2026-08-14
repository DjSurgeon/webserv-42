#!/bin/bash

echo "Content-Type: text/html"
echo ""
echo """<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Bash Script</title>
</head>
<body>
    <h1>Hello from a Bash script</h1>
    <h2>This is a random number between 1 and 100:<h2>
"""

sleep 10

echo $((RANDOM % 100 + 1))
echo "</body></html>"
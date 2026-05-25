#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r\n\r\n")
print("<html><head><title>Python CGI</title></head><body>")
print("<h1>Hello from Python CGI! 🐍</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

if os.environ.get("REQUEST_METHOD") == "POST":
    body = sys.stdin.read()
    print("<h2>POST Body Received:</h2>")
    print(f"<pre>{body}</pre>")

print("</body></html>")

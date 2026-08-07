#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

print("Content-Type: text/html")
print()

query = parse_qs(os.environ.get("QUERY_STRING", ""))
name = query.get("name", [""])[0]

if name:
    message = f"Hello {name}!"
else:
    message = "Greetings!"

print(f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Python CGI</title>
</head>
<body>
    <h1>{message}</h1>
</body>
</html>
""")
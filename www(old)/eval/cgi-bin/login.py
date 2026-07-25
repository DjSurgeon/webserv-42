#!/usr/bin/env python3
import os
import sys
import urllib.parse

print("Content-Type: text/html\r")

method = os.environ.get("REQUEST_METHOD", "GET")
auth_user = os.environ.get("AUTH_USER")

if method == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(content_length)
    parsed_body = urllib.parse.parse_qs(body)
    
    user = parsed_body.get("user", [""])[0]
    password = parsed_body.get("pass", [""])[0]
    
    if user == "admin" and password == "1234":
        # Ask C++ to create the session in RAM
        print(f"X-Create-Session: {user}\r")
        print("\r") # End of headers
        print("<html><body>")
        print("<h1>Login Exitoso!</h1>")
        print("<p>Se ha generado y enviado tu Cookie de Sesión.</p>")
        print("<a href='/eval/cgi-bin/login.py'>Volver al Dashboard</a>")
        print("</body></html>")
    else:
        print("\r") # End of headers
        print("<html><body>")
        print("<h1>Error: Credenciales Incorrectas</h1>")
        print("<a href='/eval/cgi-bin/login.py'>Reintentar</a>")
        print("</body></html>")

else:
    print("\r") # End of headers
    print("<html><body>")
    if auth_user:
        print(f"<h1>Bienvenido de nuevo al Panel de Control, {auth_user}!</h1>")
        print("<p>El Servidor C++ ha reconocido tu Sesión Activa desde la RAM.</p>")
    else:
        print("<h1>Autenticación Requerida</h1>")
        print("<form method='POST'>")
        print("Usuario: <input type='text' name='user'><br>")
        print("Clave: <input type='password' name='pass'><br>")
        print("<input type='submit' value='Entrar'>")
        print("</form>")
    print("</body></html>")

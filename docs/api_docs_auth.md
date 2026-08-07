# Documentación API de Autenticación (Session & Cookies)

Esta documentación está diseñada para que el equipo de Frontend pueda integrar el sistema nativo de login, perfiles y logout en la página web de prueba de `webserv`. Todos los endpoints se comunican directamente con el núcleo en C++ (sin pasar por CGI).

---

## 1. Login
Crea una sesión de usuario de forma segura y devuelve una cookie de sesión con una validez de 1 hora.

**Endpoint:** `POST /api/login`

### Request (Frontend)
El cliente debe enviar un payload con el nombre de usuario. El servidor buscará el parámetro `username=X` en el cuerpo del mensaje.

```http
POST /api/login HTTP/1.1
Host: localhost:8080
Content-Type: application/x-www-form-urlencoded

username=Evaluador42
```

### Response (Servidor)
El servidor inyectará la cabecera `Set-Cookie` para que el navegador del cliente la guarde automáticamente.

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 104
Set-Cookie: session_id=abc123randomstringXYZ; Path=/; Max-Age=3600

<html>...Logged in successfully as Evaluador42...</html>
```

> [!TIP]
> **Integración Frontend:** Puedes usar un formulario HTML tradicional (`<form action="/api/login" method="POST">`) que tenga un `<input type="text" name="username">`. El navegador se encargará de enviarlo en el formato correcto y guardará la cookie automáticamente.

---

## 2. Perfil / Ruta Protegida
Verifica si el usuario tiene una sesión activa y válida en el servidor. 

**Endpoint:** `GET /api/profile`

### Request (Frontend)
El navegador enviará automáticamente la cabecera `Cookie` si se guardó previamente durante el login.

```http
GET /api/profile HTTP/1.1
Host: localhost:8080
Cookie: session_id=abc123randomstringXYZ
```

### Responses (Servidor)

**Caso A: Sesión Válida (200 OK)**
El servidor reconoce el ID de la cookie, busca al usuario en la memoria RAM y muestra la web autorizada.
```http
HTTP/1.1 200 OK
Content-Type: text/html

<html>...Welcome back to your profile, Evaluador42!...</html>
```

**Caso B: Sin sesión o caducada (401 Unauthorized)**
Si el usuario no envía cookie o el `Max-Age` expiró, el servidor le denegará el acceso.
```http
HTTP/1.1 401 Unauthorized
Content-Type: text/html

<html>...401 Unauthorized... You must log in...</html>
```

> [!IMPORTANT]
> **Integración Frontend:** Si haces las llamadas vía AJAX (fetch/axios) a `/api/profile`, recuerda activar la opción `credentials: 'include'` para que JavaScript envíe las cookies, y captura el código `401` para redirigir al usuario al Login.

---

## 3. Logout
Destruye la sesión activa en el mapa de memoria del servidor C++ y envía una orden al navegador para que elimine la cookie localmente.

**Endpoint:** `POST /api/logout`

### Request (Frontend)
```http
POST /api/logout HTTP/1.1
Host: localhost:8080
Cookie: session_id=abc123randomstringXYZ
```

### Response (Servidor)
El servidor devuelve el mismo `session_id` pero vacío y con la fecha de expiración en 1970, forzando al navegador a borrarlo instantáneamente.

```http
HTTP/1.1 200 OK
Content-Type: text/html
Set-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT

<html>...Logged out successfully...</html>
```

> [!TIP]
> **Integración Frontend:** El botón de "Cerrar Sesión" de la web puede ser un mini-formulario que dispare un método POST hacia `/api/logout`, o bien un botón manejado por Javascript con un `fetch`.

# 🔄 WEBSERV - DIAGRAMAS DE FLUJO Y SECUENCIAS

---

## 1️⃣ FLUJO GENERAL DEL SERVIDOR

```
┌──────────────────────────────────────────────────────────┐
│                      MAIN                                │
│  - Parse arguments (config file path)                    │
│  - Read config file                                      │
│  - Create servers (uno por puerto+host)                  │
└────────────────┬─────────────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────────────┐
│                   EVENT LOOP                             │
│  - poll() en todos los FDs                              │
│  - Esperar evento (timeout 1 segundo)                    │
│  - Procesar eventos                                      │
└────────────────┬─────────────────────────────────────────┘
                 │
        ┌────────┴────────┐
        │                 │
        ▼                 ▼
    ¿Server FD?      ¿Client FD?
    Escucha          (read/write)
        │                 │
        ▼                 ▼
  ┌────────────┐  ┌──────────────┐
  │ accept()   │  │ Procesar     │
  │ +O_NONBLOCK│  │ petición     │
  └─────┬──────┘  └──────┬───────┘
        │                │
        ▼                ▼
   Nueva conexión   ¿Completo?
   (agregar a     
    pollfds)          │
                      ├─ NO → Esperar más datos
                      │
                      └─ SÍ → Generar respuesta
                              │
                              ▼
                         Enviar respuesta
                              │
                              ├─ SIN COMPLETAR
                              │  → Esperar socket ready POLLOUT
                              │
                              └─ COMPLETADO
                                 → Cerrar o Keep-Alive
                                 → Esperar nueva petición
```

---

## 2️⃣ CICLO DE VIDA DE UNA PETICIÓN HTTP

```
┌─────────────────────────────────────────────────────────────┐
│ CLIENTE                        SERVIDOR                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│ 1. Conecta a puerto 8080 ──────────────→ accept() new conn │
│                                         │                   │
│ 2. Envía petición               ┌──────┘ pollfds           │
│    (posiblemente en trozos) →  │        POLLIN ready      │
│                                │        │                  │
│ 3. Espera respuesta      ┌──────▼──────────────┐           │
│    (cliente bloqueado)   │ RequestParser (FSM) │           │
│                          │ parse(chunk)        │           │
│                          │ - STATE_REQUEST_LINE│           │
│                          │ - STATE_HEADERS     │           │
│                          │ - STATE_BODY        │           │
│                          │ - STATE_COMPLETE    │           │
│                          └──────┬──────────────┘           │
│                                 │                          │
│                          ¿COMPLETO? → NO → Esperar      │
│                                 │                          │
│                                 ├─ SÍ                     │
│                                 │   │                     │
│                                 ▼   │                     │
│                          ┌──────────┴────┐                │
│                          │ Response Gen  │                │
│                          │ - Routing     │                │
│                          │ - Handler     │                │
│                          │ - Procesar    │                │
│                          └────────┬──────┘                │
│                                   │                       │
│ 4. Recibe respuesta       ┌───────▼──────────┐            │
│    (en trozos) ←─────────┤ send(response)   │            │
│                          │ non-blocking     │            │
│    ├─ Parsea headers     └───────┬──────────┘            │
│    ├─ Descarga body (si aplica) │                        │
│    └─ Renderiza en navegador    │                        │
│                                  │                        │
│ 5. Cierra o Keep-Alive  ────────→ close() o             │
│                                   esperar nueva petición  │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 3️⃣ REQUEST PARSER - MÁQUINA DE ESTADOS DETALLADA

```
┌─────────────┐
│   START     │
└──────┬──────┘
       │ Recibir primer byte
       ▼
┌──────────────────────────┐
│ STATE_REQUEST_LINE       │  Leyendo: "GET / HTTP/1.1"
├──────────────────────────┤
│ buffer += recv()         │
│ buscar "\r\n"            │
│                          │
│ si encontrado:           │
│ - parsear: METHOD URI    │
│   PROTOCOL               │
│ - validar format         │
│ - cambiar a HEADERS      │
│                          │
│ si falta:                │
│ - retornar INCOMPLETE    │
└──────┬───────────────────┘
       │
       ▼
┌──────────────────────────┐
│ STATE_HEADERS            │  Leyendo headers
├──────────────────────────┤
│ Lee línea por línea      │
│ buscar "\r\n"            │
│                          │
│ si línea vacía:          │
│ - Fin de headers         │
│ - Revisar Content-Length │
│ - Si tiene body:         │
│   → STATE_BODY           │
│ - Si no tiene:           │
│   → STATE_COMPLETE       │
│                          │
│ si línea no vacía:       │
│ - parsear "Key: Value"   │
│ - guardar en map         │
│ - volver a leer          │
└──────┬───────────────────┘
       │
       ├──── NO body ───→ ┌──────────────────┐
       │                 │ STATE_COMPLETE   │
       │                 │ retornar COMPLETE│
       │                 └──────────────────┘
       │
       └──── Con body ──→ ┌──────────────────────────┐
                         │ STATE_BODY               │
                         ├──────────────────────────┤
                         │ expected = Content-Length│
                         │ received = 0             │
                         │                         │
                         │ mientras received <      │
                         │ expected:                │
                         │ - leer chunk            │
                         │ - agregar a body        │
                         │ - received += chunk.len │
                         │                         │
                         │ si received == expected:│
                         │ - STATE_COMPLETE        │
                         │ - retornar COMPLETE     │
                         │                         │
                         │ si error en parsing:    │
                         │ - STATE_ERROR           │
                         │ - retornar ERROR        │
                         └──────────────────────────┘

ERROR State (en cualquier momento):
- Fallo en formato HTTP
- URI demasiada larga
- Headers malformados
- Content-Length inconsistente
→ retornar PARSE_ERROR
```

---

## 4️⃣ ROUTING: ENCONTRAR SERVER + LOCATION

```
┌────────────────────────────────────────┐
│ Request llega                          │
│ - port: 8080                          │
│ - headers.Host: "example.com"         │
│ - uri: "/upload/image.jpg"            │
└────────────┬─────────────────────────┘
             │
             ▼
┌────────────────────────────────────────┐
│ Config tiene 3 servers:                │
├────────────────────────────────────────┤
│ Server A: port=8080, names=[example]   │ ← MATCH
│ Server B: port=8081, names=[api]       │
│ Server C: port=8080, names=[other]     │
└────────────┬─────────────────────────┘
             │ Buscar por puerto + server_name
             ▼
┌────────────────────────────────────────┐
│ RESULTADO: Server A                    │
│ (puerto 8080 + nombre "example.com")   │
└────────────┬─────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────┐
│ Server A tiene locations:                  │
├────────────────────────────────────────────┤
│ location "/"          (length=1)           │
│ location "/upload"    (length=7)  ← MEJOR │
│ location "/api"       (length=4)           │
│                                            │
│ Request URI: "/upload/image.jpg"           │
│ ¿Longest prefix match?                     │
│ - "/" : /upload/image.jpg.find("/")=0 ✓   │
│ - "/upload" : /upload/image.jpg.find("/upload")=0 ✓
│ - "/api" : /upload/image.jpg.find("/api")=npos ✗
│                                            │
│ Ganador: "/upload" (length 7)              │
└────────────┬─────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────┐
│ RESULTADO: Location "/upload"          │
│ - allowed_methods: [POST, GET]         │
│ - upload_path: "/var/www/uploads"      │
│ - client_max_body_size: 5MB            │
└────────────┬─────────────────────────┘
             │
             ▼
┌────────────────────────────────────────┐
│ Validaciones:                          │
├────────────────────────────────────────┤
│ ✓ Método permitido? (GET en lista)     │
│ ✓ Tamaño body < limit? (sí)            │
│ ✓ ¿Existe archivo/directorio?          │
│ ✓ ¿Tienes permisos de lectura?         │
│                                        │
│ TODO OK → Proceder                     │
│ ERROR → Retornar 403/404/413           │
└────────────┬─────────────────────────┘
             │
             ▼
┌────────────────────────────────────────┐
│ Seleccionar Handler:                   │
├────────────────────────────────────────┤
│ - ¿Es CGI? (.php, .py)                 │
│   → CgiHandler                         │
│ - ¿Es upload? (location /upload)       │
│   → UploadHandler                      │
│ - ¿Es DELETE?                          │
│   → DeleteHandler                      │
│ - ¿Es directorio?                      │
│   → AutoIndexHandler                   │
│ - ¿Es archivo?                         │
│   → StaticHandler                      │
│                                        │
│ En este caso: StaticHandler            │
│ (archivo "/var/www/uploads/image.jpg") │
└────────────┬─────────────────────────┘
             │
             ▼
         GENERAR RESPUESTA
```

---

## 5️⃣ CGI EXECUTION FLOW

```
┌──────────────────────────┐
│ Request: GET /app/index.php?name=John
│ Location: /app (CGI enabled)
│ cgi_path: /usr/bin/php-cgi
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│ CgiHandler::handle()                 │
├──────────────────────────────────────┤
│ 1. Preparar entorno CGI              │
│    env[0] = "REQUEST_METHOD=GET"     │
│    env[1] = "QUERY_STRING=name=John" │
│    env[2] = "SCRIPT_NAME=/app/index" │
│    env[3] = "SCRIPT_FILENAME=/path"  │
│    env[4] = "SERVER_NAME=localhost"  │
│    env[5] = "SERVER_PORT=8080"       │
│    env[6] = "SERVER_PROTOCOL=HTTP/1.1"
│    ... (más variables)               │
│                                      │
│ 2. Convertir a char** para execve    │
│                                      │
│ 3. fork()                            │
└────────────┬─────────────────────────┘
             │
    ┌────────┴────────┐
    │                 │
    ▼ PID==0         ▼ PID>0
    HIJO            PADRE
    │               │
    ├─ Redirigir   ├─ epoll/poll WUNOHANG
    │  stdin ──→   │  waitpid(pid, &status, WNOHANG)
    │  stdout      │  │
    │  (dup2)      │  └─ Si no terminó:
    │              │     Registrar en pending
    ├─ chdir()     │
    │  a root      └─ Cuando vuelve:
    │              │  Leer salida
    ├─ execve()    │  Construir respuesta
    │  php-cgi     │  Enviar al cliente
    │              │
    └─ exit(127)   │
                   │
                   Respuesta HTTP:
                   200 OK
                   Content-Type: text/html
                   Content-Length: [tamaño salida]
                   
                   [HTML generado por PHP]
```

---

## 6️⃣ FILE UPLOAD (MULTIPART) PARSING

```
POST /upload HTTP/1.1
Host: localhost:8080
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary
Content-Length: 250

------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="test.txt"
Content-Type: text/plain

[CONTENIDO DEL ARCHIVO]
------WebKitFormBoundary--

┌──────────────────────────────────────┐
│ UploadHandler::handle()              │
├──────────────────────────────────────┤
│ 1. Parsear boundary de header        │
│    "----WebKitFormBoundary"          │
│                                      │
│ 2. Split body por boundary           │
│    Partes:                           │
│    ├─ Content-Disposition header     │
│    ├─ Nombre del campo               │
│    ├─ Filename                       │
│    └─ Contenido binario              │
│                                      │
│ 3. Extraer filename                  │
│    "test.txt"                        │
│                                      │
│ 4. Extraer contenido                 │
│    [CONTENIDO DEL ARCHIVO]           │
│                                      │
│ 5. Guardar a disco                   │
│    open("/var/www/uploads/test.txt") │
│    write(content)                    │
│    close()                           │
│                                      │
│ 6. Retornar 200 OK o error           │
└──────────────────────────────────────┘
```

---

## 7️⃣ TIMINGS Y DELAYS

```
┌─────────────────────────────────────────────────┐
│            TIMELINE DE UN REQUEST                │
└─────────────────────────────────────────────────┘

T+0ms:   Client conecta
         └─ accept() → nueva conexión
         └─ agregar a pollfds

T+10ms:  Cliente envía primeros bytes
         └─ poll() devuelve POLLIN ready
         └─ read() obtiene "GET / HTTP/1.1\r\n"
         └─ RequestParser procesa

T+50ms:  Cliente envía resto de headers
         └─ poll() devuelve POLLIN ready
         └─ read() obtiene headers
         └─ RequestParser completa COMPLETE

T+60ms:  Servidor procesa request
         └─ Routing
         └─ Handler selecciona StaticHandler
         └─ Lee archivo del disco
         └─ Construye respuesta HTTP

T+65ms:  Socket listo para escribir
         └─ poll() devuelve POLLOUT ready
         └─ send() envía respuesta

T+100ms: Respuesta completada
         └─ Cliente recibe y procesa
         └─ Renderiza en navegador

TOTAL: ~100ms (típico para archivo pequeño)

Si es CGI (PHP):
T+0ms:   Conecta
T+50ms:  Request completo
T+60ms:  fork() + execve()
T+500ms: PHP ejecuta (simulado)
T+510ms: send() respuesta
T+550ms: Cliente recibe

TOTAL: ~550ms (dependiendo del script)
```

---

## 8️⃣ ESTADO DE MEMORIA

```
┌──────────────────────────────────────────────────┐
│ Antes de iniciar servidor                        │
├──────────────────────────────────────────────────┤
│ Config                      2 KB (parsed config) │
│ EventLoop                   4 KB (estructura)    │
│ ServerSockets (3 puertos)   12 KB                │
│                                                  │
│ Total: ~20 KB                                    │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ Durante request típico (10 clientes)            │
├──────────────────────────────────────────────────┤
│ Config                      2 KB                │
│ EventLoop                   4 KB                │
│ ServerSockets               12 KB               │
│ ClientConnections (10)      50 KB (5 KB c/u)   │
│   - Request buffers: 20 KB                      │
│   - Response buffers: 30 KB                     │
│                                                 │
│ Total: ~68 KB                                   │
│                                                 │
│ ¡Sin memoria dinámica ilimitada!                │
│ ¡Sin memory leaks!                              │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ Con 100 clientes simultáneos                     │
├──────────────────────────────────────────────────┤
│ Base                        20 KB               │
│ ClientConnections (100)     500 KB (5 KB c/u)  │
│   - Request buffers: 200 KB                     │
│   - Response buffers: 300 KB                    │
│                                                 │
│ Total: ~520 KB                                  │
│                                                 │
│ ¡Totalmente eficiente!                          │
│ ¡Menor uso que un servidor threading!           │
└──────────────────────────────────────────────────┘
```

---

## 9️⃣ MANEJO DE ERRORES - DECISION TREE

```
Request llega
│
├─ ¿Puedo parsear?
│  ├─ SÍ → Continuar
│  └─ NO → 400 Bad Request
│
├─ ¿URI muy larga?
│  ├─ SÍ → 414 URI Too Long
│  └─ NO → Continuar
│
├─ ¿Content-Length > limit?
│  ├─ SÍ → 413 Payload Too Large
│  └─ NO → Continuar
│
├─ ¿Hay server para este puerto?
│  ├─ SÍ → Continuar
│  └─ NO → 400 Bad Request
│
├─ ¿Host válido? (Host header)
│  ├─ SÍ → Continuar
│  └─ NO → 400 Bad Request
│
├─ ¿Location existe?
│  ├─ SÍ → Continuar
│  └─ NO → 404 Not Found
│
├─ ¿Método permitido?
│  ├─ SÍ → Continuar
│  └─ NO → 405 Method Not Allowed
│
├─ ¿Ruta dentro de root?
│  ├─ SÍ → Continuar
│  └─ NO → 403 Forbidden
│
├─ ¿Archivo/directorio existe?
│  ├─ SÍ → Continuar
│  └─ NO → 404 Not Found
│
├─ ¿Permisos de lectura?
│  ├─ SÍ → Continuar
│  └─ NO → 403 Forbidden
│
├─ ¿Es CGI? (extensión .php, .py)
│  ├─ SÍ → CgiHandler
│  │  ├─ ¿fork() exitoso?
│  │  │  ├─ SÍ → Esperar salida
│  │  │  └─ NO → 502 Bad Gateway
│  │  └─ ¿Timeout en CGI?
│  │     ├─ SÍ → 504 Gateway Timeout
│  │     └─ NO → Procesar salida
│  │
│  └─ NO → Continuar
│
├─ ¿Es directorio?
│  ├─ SÍ
│  │  ├─ ¿Hay archivo index?
│  │  │  ├─ SÍ → Servir index
│  │  │  └─ NO
│  │  │     ├─ ¿Autoindex ON?
│  │  │     │  ├─ SÍ → Generar HTML
│  │  │     │  └─ NO → 403 Forbidden
│  │  │
│  │  └─ ¿Request termina con /?
│  │     ├─ NO → 301 Redirect a /
│  │     └─ SÍ → OK
│  │
│  └─ NO → Es archivo
│
├─ ¿Es upload (POST)?
│  ├─ SÍ → UploadHandler
│  │  ├─ ¿Parse multipart OK?
│  │  │  ├─ SÍ → Guardar archivo
│  │  │  └─ NO → 400 Bad Request
│  │  └─ ¿Permisos de escritura?
│  │     ├─ SÍ → 200 OK o 201 Created
│  │     └─ NO → 403 Forbidden
│  │
│  └─ NO → Continuar
│
├─ ¿Es DELETE?
│  ├─ SÍ → DeleteHandler
│  │  ├─ ¿Existe archivo?
│  │  │  ├─ SÍ → Eliminar
│  │  │  └─ NO → 404 Not Found
│  │  └─ ¿Éxito?
│  │     ├─ SÍ → 204 No Content o 200 OK
│  │     └─ NO → 403 Forbidden
│  │
│  └─ NO → GET (archivo estático)
│
└─ 200 OK
   + Content-Type correcto
   + Content-Length correcto
   + Headers CRLF correcto
   + Body

ERROR GENÉRICO:
Algo inesperado → 500 Internal Server Error
```

---

## 🔟 SECUENCIA COMPLETA DE MAIN()

```cpp
main()
│
├─ Parse arguments
│  └─ config_file = argv[1] o "default.conf"
│
├─ ConfigReader::parse(config_file)
│  ├─ Leer archivo
│  ├─ Parsear bloques server
│  ├─ Parsear locations dentro de servers
│  └─ Retornar std::vector<Server>
│
├─ Para cada Server:
│  ├─ ServerSocket::bind(port, host)
│  │  ├─ socket() → crear socket
│  │  ├─ setsockopt() → reusar puerto
│  │  ├─ bind() → asociar a puerto
│  │  ├─ listen() → marcar como servidor
│  │  └─ fcntl(O_NONBLOCK) → non-blocking
│  │
│  └─ EventLoop::addServerSocket(fd)
│
├─ EventLoop::run()
│  └─ Loop infinito:
│     ├─ poll(pollfds, timeout)
│     ├─ Para cada FD ready:
│     │  ├─ Si server FD: accept() → nuevo cliente
│     │  ├─ Si client FD + POLLIN: read() + parse
│     │  └─ Si client FD + POLLOUT: send() response
│     └─ Revisar timeouts, limpiar conexiones muertas
│
└─ exit(0)

EN CASO DE ERROR:
├─ No puedo abrir config → exit(1)
├─ No puedo hacer bind → exit(1)
├─ Signal (Ctrl+C) → Cleanup y exit(0)
└─ Excepción no capturada → exit(1)
```

---

## 1️⃣1️⃣ RESPONSABILIDADES POR CLASE

```
main.cpp
├─ Parse argv
├─ Read config
└─ Start EventLoop

ConfigReader
├─ Leer archivo .conf
├─ Parsear bloques server
├─ Parsear bloques location
└─ Retornar std::vector<Server>

Server
├─ Almacenar configuración
├─ Encontrar Location (longest prefix)
└─ Validar request

Location
├─ Ruta específica
├─ Métodos permitidos
├─ Configuración CGI
└─ Upload path

EventLoop
├─ Mantener pollfds
├─ poll() loop
├─ Aceptar conexiones
├─ Despachar eventos
└─ Cleanup

ClientConnection
├─ Almacenar Request
├─ Almacenar Response
├─ RequestParser
└─ Estado de la conexión

RequestParser
├─ FSM parsing
├─ Almacenar Request
└─ Validación

Request
├─ Método (GET, POST, DELETE)
├─ URI
├─ Headers (map)
└─ Body

Response
├─ Encontrar Server + Location
├─ Routing
├─ Seleccionar Handler
└─ Generar respuesta HTTP

Handlers
├─ StaticHandler: Archivos estáticos
├─ AutoIndexHandler: Directory listing
├─ CgiHandler: fork + execve
├─ UploadHandler: Guardar archivos
├─ DeleteHandler: Eliminar archivos
└─ RedirectHandler: Redirigir

Utils
├─ Logger: Debug output
├─ FileUtils: Operaciones archivo
├─ StringUtils: Manipulación strings
├─ MimeTypes: Content-Type mapping
├─ PathUtils: Normalización rutas
└─ TimeoutManager: Detectar inactivos
```

¡Estos diagramas te dan la visión completa del proyecto! 🚀

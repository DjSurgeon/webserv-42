# ⚠️ WEBSERV - BUENAS PRÁCTICAS & ANTIPATRONES

> Lo que SÍ debes hacer y lo que NO debes hacer en tu webserv

---

## 🟢 BUENAS PRÁCTICAS

### 1. PARSING HTTP - MÁQUINA DE ESTADOS

✅ **BIEN:**
```cpp
// RequestParser con máquina de estados
// Procesa byte a byte, maneja fragmentación
enum State { REQUEST_LINE, HEADERS, BODY, COMPLETE };

switch(current_state) {
    case REQUEST_LINE:
        // Buscar \r\n
        if (buffer.find("\r\n") != npos) {
            // Parsear y cambiar estado
            current_state = HEADERS;
        }
        break;
    // ... más estados
}
```

✅ **Por qué:**
- Maneja peticiones que llegan en trozos pequeños
- No asume que todo viene de golpe
- Rápido y eficiente
- Soporta chunked encoding fácilmente

❌ **MAL:**
```cpp
// Anti-patrón: leer y asumir petición completa
std::string full_request;
read(socket, buffer, BUFFER_SIZE);
full_request = buffer;  // ¡Esperar más datos!
parse(full_request);    // Falla si no está completo
```

---

### 2. NON-BLOCKING I/O

✅ **BIEN:**
```cpp
// 1. Configurar socket como non-blocking
fcntl(socket_fd, F_SETFL, O_NONBLOCK);

// 2. Usar poll/epoll/select para monitorizar
poll(pollfds, nfds, timeout);

// 3. Leer solo cuando poll dice que hay datos
if (pollfds[i].revents & POLLIN) {
    bytes_read = read(fd, buffer, size);
    // bytes_read > 0: datos leídos
    // bytes_read == 0: conexión cerrada
    // bytes_read == -1: error (pero no bloqueante)
}

// 4. NO revisar errno
// ✅ Revisar solo el retorno de read/write
```

✅ **Por qué:**
- El servidor NUNCA se bloquea esperando a un cliente lento
- Soporta 100+ conexiones simultáneas
- Eficiente en recursos

❌ **MAL:**
```cpp
// Anti-patrón 1: Bloqueo directo
read(socket, buffer, size);  // Se queda aquí si no hay datos
process(buffer);              // Nunca llega si el cliente es lento

// Anti-patrón 2: Revisar errno
read(socket, buffer, size);
if (errno == EAGAIN) {        // PROHIBIDO por el subject!
    // ...
}

// Anti-patrón 3: Mezclar blocking y non-blocking
fcntl(socket, F_SETFL, O_NONBLOCK);
// ... pero luego usar funciones que asumen blocking
```

---

### 3. MULTIPLICACIÓN DE CONEXIONES

✅ **BIEN:**
```cpp
class EventLoop {
    std::vector<pollfd> pollfds;
    std::map<int, ClientConnection*> connections;
    
    void run() {
        while (running) {
            int nready = poll(pollfds, pollfds.size(), TIMEOUT);
            
            for (size_t i = 0; i < pollfds.size(); ++i) {
                if (pollfds[i].revents == 0) continue;
                
                int fd = pollfds[i].fd;
                
                if (pollfds[i].revents & POLLIN) {
                    handleRead(fd);
                }
                if (pollfds[i].revents & POLLOUT) {
                    handleWrite(fd);
                }
            }
        }
    }
};
```

✅ **Por qué:**
- Un único bucle para TODAS las conexiones
- poll() duerme el proceso (eficiente)
- No hay race conditions (sin threads)
- Fácil de debuggear

❌ **MAL:**
```cpp
// Anti-patrón 1: Un thread por cliente
while (accept_connection) {
    pthread_create(&thread, NULL, handle_client, socket);
    // ¡Explosión de threads en stress test!
}

// Anti-patrón 2: Bucle bloqueante
while (true) {
    socket_client = accept(server_socket);  // BLOQUEANTE
    handle_request(socket_client);           // BLOQUEANTE
    // No atiende otros clientes
}

// Anti-patrón 3: Múltiples poll() calls
poll(read_fds, ...);   // Primera ronda
poll(write_fds, ...);  // Segunda ronda
// Ineficiente, duplica trabajo
```

---

### 4. GESTIÓN DE MEMORIA

✅ **BIEN:**
```cpp
// RAII: Resource Acquisition Is Initialization
class ClientConnection {
public:
    ClientConnection(int fd) : socket_fd(fd) {
        fcntl(fd, F_SETFL, O_NONBLOCK);
    }
    
    ~ClientConnection() {
        close(socket_fd);  // Siempre cerrar en destructor
    }
    
private:
    int socket_fd;
};

// Uso:
{
    ClientConnection conn(new_socket);
    // Automáticamente se cierra cuando sale del scope
}
```

✅ **Por qué:**
- Sin memory leaks
- Código más limpio
- Excepciones seguras

❌ **MAL:**
```cpp
// Anti-patrón 1: Olvidar close()
int socket = accept(...);
handle_request(socket);
// ¡Nunca cierro! → Leak de file descriptors

// Anti-patrón 2: Liberar mal
char* buffer = malloc(1024);
read(socket, buffer, 1024);
free(buffer);  // ¡Sigue siendo un descriptor abierto!

// Anti-patrón 3: No limpiar en errores
socket = accept(...);
if (error) {
    return;  // ¡No cierro el socket!
}
close(socket);  // Nunca se ejecuta
```

---

### 5. PARSING DE CONFIGURACIÓN

✅ **BIEN:**
```cpp
// Configuración jerárquica con herencia
class Context {
protected:
    std::string root;
    std::vector<std::string> index_files;
    // ...
};

class Location : public Context {
    // Hereda root, index_files, etc.
    // Puede sobrescribir si es necesario
};

class Server : public Context {
    std::vector<Location> locations;
    // Cada Location hereda de Server
};

// Uso:
Server server;
server.setRoot("/var/www");
server.addIndexFile("index.html");

Location location("/upload");
location.addAllowedMethod("POST");
location.setUploadPath("/tmp/uploads");
server.addLocation(location);

// Si Location no define root, hereda de Server
```

✅ **Por qué:**
- DRY (Don't Repeat Yourself)
- Fácil de mantener
- Similar a Nginx (familiar)

❌ **MAL:**
```cpp
// Anti-patrón: Duplicar configuración
struct ServerConfig {
    std::string root;
    std::vector<std::string> index;
    // ... 20 campos más
};

struct LocationConfig {
    std::string root;  // ¡Duplicado!
    std::vector<std::string> index;  // ¡Duplicado!
    // ... código repetido
};

// Cambiar root requiere hacerlo en 10 lugares
```

---

### 6. ROUTING CON LONGEST PREFIX MATCH

✅ **BIEN:**
```cpp
// Longest Prefix Match (como Nginx)
Location* findLocation(const std::string& path) {
    Location* best = NULL;
    size_t best_len = 0;
    
    // Iterar todas las locations
    for (Location& loc : locations) {
        const std::string& loc_path = loc.getPath();
        
        // ¿Esta location coincide con el path?
        if (path.find(loc_path) == 0) {
            // ¿Es la más específica (longest match)?
            if (loc_path.length() > best_len) {
                best = &loc;
                best_len = loc_path.length();
            }
        }
    }
    
    return best;
}

// Ejemplo:
// Request: /upload/avatar/photo.jpg
// Locations:
//   /            → length 1
//   /upload      → length 7 ← GANADOR
//   /upload/avatar → length 14 ← SI EXISTE, GANADOR
```

✅ **Por qué:**
- Intuitivo para usuarios
- Eficiente
- Comportamiento predecible

❌ **MAL:**
```cpp
// Anti-patrón 1: Primera coincidencia
Location* findLocation(const std::string& path) {
    for (Location& loc : locations) {
        if (path.find(loc.getPath()) == 0) {
            return &loc;  // ¡Primera que coincide, no la mejor!
        }
    }
}

// Anti-patrón 2: Regex (no permitido)
if (std::regex_match(path, location_regex)) {
    // ¡El subject dice NO regex!
}
```

---

### 7. CGI EXECUTION

✅ **BIEN:**
```cpp
class CgiHandler {
    Response handle(const Request& req, const Location& loc) {
        // 1. Preparar variables de entorno CGI
        std::vector<std::string> env;
        env.push_back(std::string("REQUEST_METHOD=") + req.getMethod());
        env.push_back(std::string("CONTENT_LENGTH=") + 
                      req.getHeader("Content-Length"));
        env.push_back(std::string("QUERY_STRING=") + req.getQueryString());
        // ... más variables
        
        // 2. Convertir a char** para execve
        char** envp = new char*[env.size() + 1];
        for (size_t i = 0; i < env.size(); ++i) {
            envp[i] = const_cast<char*>(env[i].c_str());
        }
        envp[env.size()] = NULL;
        
        // 3. fork()
        pid_t pid = fork();
        if (pid == 0) {
            // HIJO: redirigir entrada/salida
            dup2(input_fd, STDIN_FILENO);   // stdin desde archivo
            dup2(output_fd, STDOUT_FILENO);  // stdout a archivo
            
            // Cambiar directorio para rutas relativas
            chdir(loc.getRoot().c_str());
            
            // Ejecutar CGI
            execve(cgi_path.c_str(), args, envp);
            
            // Si execve falla, terminar
            exit(127);
        } else if (pid > 0) {
            // PADRE: esperar a hijo (no bloqueante con epoll)
            int status;
            waitpid(pid, &status, 0);
            
            // Leer salida del archivo
            std::string output = readFile(output_file);
            
            return Response(200, output);
        } else {
            return Response(500, "Fork failed");
        }
    }
};
```

✅ **Por qué:**
- Ejecuta scripts CGI correctamente
- Pasa variables de entorno
- Captura entrada/salida
- Maneja errores

❌ **MAL:**
```cpp
// Anti-patrón 1: system()
system("php-cgi script.php");  // ¡Inseguro!

// Anti-patrón 2: Olvidar envp
execve(cgi_path, args, NULL);  // CGI recibe envp vacío

// Anti-patrón 3: Bloqueo en waitpid()
pid_t pid = fork();
if (pid > 0) {
    waitpid(pid, &status, 0);  // Bloqueante en event loop
    // El servidor se congela esperando CGI
}

// Anti-patrón 4: No cambiar de directorio
// CGI hace: include("./template.html")
// Pero ¿de dónde es "./"? ¡Relativo a dónde se ejecuta!
// Debe ser relativo a loc.getRoot()
```

---

### 8. MANEJO DE ERRORES

✅ **BIEN:**
```cpp
try {
    // Parsing
    ParseStatus status = parser.parse(data);
    if (status == PARSE_ERROR) {
        throw HttpException(400, "Bad Request");
    }
    
    Request req = parser.getRequest();
    
    // Validación
    if (req.getUri().length() > 2048) {
        throw HttpException(414, "URI Too Long");
    }
    
    if (req.getBody().length() > config.getMaxBodySize()) {
        throw HttpException(413, "Payload Too Large");
    }
    
    // Routing
    Server* server = config.findServer(port, req.getHeader("Host"));
    if (!server) {
        throw HttpException(400, "Bad Host");
    }
    
    Location* loc = server->findLocation(req.getUri());
    if (!loc || !loc->isMethodAllowed(req.getMethod())) {
        throw HttpException(405, "Method Not Allowed");
    }
    
    // Generación
    Response response;
    response.generate(req, *server, *loc);
    
    return response;
    
} catch (const HttpException& e) {
    // Devolver error apropiado
    return Response(e.getStatusCode(), getErrorPage(e.getStatusCode()));
} catch (const std::exception& e) {
    return Response(500, "Internal Server Error");
}
```

✅ **Por qué:**
- Código limpio
- Fácil de leer
- Cada error es distinto
- Mensajes significativos

❌ **MAL:**
```cpp
// Anti-patrón 1: if/else infinito
if (parse_ok) {
    if (route_ok) {
        if (perm_ok) {
            if (file_ok) {
                // ... 10 levels de nesting
            } else {
                return 500;
            }
        } else {
            return 403;
        }
    } else {
        return 404;
    }
} else {
    return 400;
}

// Anti-patrón 2: No diferenciar errores
if (!something) {
    return Response(500, "Error");  // ¡Demasiado genérico!
}

// Anti-patrón 3: Crash en error
std::string root = server->getRoot();  // ¿Y si server es NULL?
return readFile(root + req.getUri());  // SEGFAULT

// Anti-patrón 4: No validar tamaños
read(socket, buffer, 1024);
std::string request = buffer;
parse(request);  // ¿Y si son 10KB? Desbordamiento
```

---

### 9. VALIDACIÓN DE RUTAS (DIRECTORY TRAVERSAL)

✅ **BIEN:**
```cpp
std::string normalizePath(const std::string& path) {
    std::string result = path;
    size_t pos = 0;
    
    // Reemplazar "//" por "/"
    while ((pos = result.find("//")) != std::string::npos) {
        result.erase(pos, 1);
    }
    
    // Eliminar "/."
    pos = 0;
    while ((pos = result.find("/.")) != std::string::npos) {
        result.erase(pos, 2);
    }
    
    // Eliminar "/.."
    pos = 0;
    while ((pos = result.find("/..")) != std::string::npos) {
        // Remover el directorio anterior también
        if (pos == 0) {
            result.erase(pos, 3);
        } else {
            size_t prev = result.rfind('/', pos - 1);
            if (prev != std::string::npos) {
                result.erase(prev, pos - prev + 3);
            }
        }
    }
    
    return result;
}

// Uso:
std::string final_path = root + normalizePath(request_path);
// "/var/www" + normalizePath("/../../../etc/passwd")
// = "/var/www" + "/etc/passwd"
// ¡SEGURO!
```

✅ **Por qué:**
- Evita ataques directory traversal
- Ruta final siempre está bajo root

❌ **MAL:**
```cpp
// Anti-patrón 1: No validar
std::string path = root + request_path;
return readFile(path);
// Request: /../../../etc/passwd
// Resultado: /var/www/../../../etc/passwd = /etc/passwd ¡EXPUESTO!

// Anti-patrón 2: Validación incompleta
if (path.find("..") == std::string::npos) {
    return readFile(path);
}
// Pero ¿y "/.../"? ¿Y "%2e%2e"? Incompleto

// Anti-patrón 3: Usar realpath incorrectamente
char resolved[PATH_MAX];
realpath(path.c_str(), resolved);
if (strncmp(resolved, root, strlen(root)) != 0) {
    // Error
}
// realpath() falla si el archivo no existe
```

---

### 10. HEADERS HTTP

✅ **BIEN:**
```cpp
// Almacenar headers en map (case-insensitive sería ideal)
std::map<std::string, std::string> headers;

// Obtener con normalización
std::string getHeader(const std::string& key) {
    // Normalizar: "Content-Type" → "content-type"
    std::string normalized = toLowerCase(key);
    
    std::map<std::string, std::string>::iterator it = headers.find(normalized);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

// Respuesta con headers correctos
std::string response;
response += "HTTP/1.1 200 OK\r\n";
response += "Content-Type: text/html\r\n";
response += "Content-Length: 1024\r\n";
response += "Connection: keep-alive\r\n";
response += "Server: webserv/1.0\r\n";
response += "\r\n";  // Línea vacía = fin de headers
response += body;
```

✅ **Por qué:**
- Headers correctos
- Navegador entiende la respuesta
- Soporta keep-alive

❌ **MAL:**
```cpp
// Anti-patrón 1: Headers inconsistentes
response = "HTTP/1.1 200 OK\n";           // ¡CRLF!
response += "Content-Type: text/html\n";  // ¡CRLF!
response += "\n";                          // ¡LF solo!
response += body;

// Anti-patrón 2: Content-Length incorrecto
std::string body = "Hello";
response += "Content-Length: 10\r\n";  // ¡Debería ser 5!

// Anti-patrón 3: Olvidar headers esenciales
response = "HTTP/1.1 200 OK\r\n";  // ¡Sin Content-Length!
response += "\r\n";
response += body;
// Navegador no sabe cuándo termina la respuesta

// Anti-patrón 4: Headers duplicados
response += "Content-Type: text/html\r\n";
response += "Content-Type: application/json\r\n";  // ¡Conflicto!
```

---

## 🔴 ANTIPATRONES

| Antipatrón | Problema | Solución |
|-----------|----------|----------|
| Asumir petición completa | Falla con datos fragmentados | FSM parser |
| Bloqueo en read/write | Servidor se congela | Non-blocking + poll |
| Revisar errno | Prohibido por subject | Revisar retorno |
| Un thread por cliente | Explosión de threads | Un loop con poll |
| fork() en loop principal | Bloquea servidor | Ejecutar asincronía |
| No cerrar FDs | Leak de descriptores | RAII + destructor |
| Duplicar configuración | Código difícil mantener | Herencia |
| Primera match routing | Comportamiento incorrecto | Longest prefix |
| system() para CGI | Inseguro | fork + execve |
| if/else infinito | Código ilegible | Excepciones |
| No validar rutas | Directory traversal | normalizePath() |
| Headers inconsistentes | Navegador no entiende | CRLF everywhere |
| Content-Length incorrecto | Cliente no sabe fin | Calcular bien |

---

## 🧪 TESTING CHECKLIST

```bash
# Básico
curl http://localhost:8080/
curl http://localhost:8080/index.html

# Métodos
curl -X POST -d "data=test" http://localhost:8080/
curl -X DELETE http://localhost:8080/file.txt

# Headers
curl -H "Host: example.com" http://localhost:8080/
curl -H "Content-Type: application/json" http://localhost:8080/

# Errores
curl http://localhost:8080/nonexistent  # 404
curl -X INVALID http://localhost:8080/  # 405
curl -H "Host: /etc/passwd" http://localhost:8080/  # Error

# Size
curl --data "$(python -c 'print("A" * 10000000)')" http://localhost:8080/
# Debería dar 413 (Payload Too Large)

# Fragmentado
echo -ne "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | \
  nc localhost 8080

# Stress
ab -n 1000 -c 100 http://localhost:8080/
hey -z 30s http://localhost:8080/

# Valgrind (memory leaks)
valgrind --leak-check=full ./webserv

# CGI
curl http://localhost:8080/script.php

# Upload
curl -F "file=@test.txt" http://localhost:8080/upload

# Directory traversal
curl http://localhost:8080/../../../../etc/passwd
# Debe dar 404 o estar dentro de root
```

---

## 📚 REFERENCIAS

- RFC 7230: HTTP/1.1 Message Syntax
- RFC 3875: CGI 1.1
- Nginx Documentation
- The Linux Programming Interface (Michael Kerrisk)

¡Evita estos problemas y tendrás un excelente webserv! 🚀

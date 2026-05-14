# 🚀 WEBSERV - PLAN MAESTRO DE ARQUITECTURA

> Consolidación de las 5 mejores implementaciones analizadas

---

## 📋 ÍNDICE EJECUTIVO

Este plan combina:
- **Webserver-1 (FuryWeb)**: Excelente RequestParser FSM + select()
- **Webserver-2 (WebservAchraf)**: Poll multiplexing + arquitectura jerárquica
- **Webserver-3**: Validación robusta + manejo de errores
- **Webserver-4 (Cluster)**: Separación clara de responsabilidades + RequestConfig dinámica
- **Webserver-5 (epoll)**: Mejor multiplexación + herencia de contexto + excepciones

---

## 🏗️ ARQUITECTURA GENERAL

```
┌─────────────────────────────────────────────────────────┐
│                   MAIN / ENTRY POINT                    │
│              (main.cpp, argument parsing)               │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│           CONFIG READER / PARSER                        │
│     (Lee archivo .conf, valida, crea objetos)          │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│                  EVENT LOOP                             │
│         (poll/epoll/kqueue - Multiplexación)           │
│  - Registra sockets de escucha (servers)               │
│  - Monitoriza sockets de clientes (read/write)         │
│  - Despachador de eventos                              │
└────────────────┬────────────────────────────────────────┘
                 │
        ┌────────┴────────┐
        ▼                 ▼
    NEW CONN?        CLIENT DATA?
        │                 │
        ▼                 ▼
   ┌─────────────┐   ┌──────────────────┐
   │ ACCEPT()    │   │ REQUEST PARSER   │
   │ NEW CLIENT  │   │ (FSM - Estados)  │
   └──────┬──────┘   └────────┬─────────┘
          │                   │
          │                   ▼
          │          ┌──────────────────┐
          │          │ REQUEST COMPLETE?│
          │          └────────┬─────────┘
          │                   │ YES
          │                   ▼
          └──────────┬────┐──────────────────────┐
                     ▼    ▼                      ▼
              ┌────────────────────────────────────────┐
              │     RESPONSE GENERATOR (Handler)       │
              │  - Routing (Server + Location)         │
              │  - Permission Check                    │
              │  - Dispatch a handler específico:      │
              │    * Static File Handler               │
              │    * Directory Listing (Autoindex)     │
              │    * CGI Handler                       │
              │    * Upload Handler                    │
              │    * Delete Handler                    │
              │    * Redirect Handler                  │
              └────────┬───────────────────────────────┘
                       │
                       ▼
              ┌──────────────────┐
              │ SEND RESPONSE    │
              │ (Non-blocking)   │
              └──────────────────┘
```

---

## 📦 ESTRUCTURA DE CARPETAS RECOMENDADA

```
webserv/
├── Makefile
├── src/
│   ├── main.cpp                    # Entry point
│   ├── config/
│   │   ├── Config.cpp/hpp          # Clase de configuración global
│   │   ├── ConfigReader.cpp/hpp    # Parser de .conf
│   │   ├── Server.cpp/hpp          # Bloque server
│   │   ├── Location.cpp/hpp        # Bloque location
│   │   └── Context.cpp/hpp         # Base común (herencia)
│   ├── network/
│   │   ├── EventLoop.cpp/hpp       # Multiplicador (poll/epoll)
│   │   ├── ServerSocket.cpp/hpp    # Socket de escucha
│   │   ├── ClientConnection.cpp/hpp# Conexión cliente
│   │   └── SocketUtils.cpp/hpp     # Helpers (bind, listen, etc.)
│   ├── http/
│   │   ├── Request.cpp/hpp         # Objeto Request
│   │   ├── RequestParser.cpp/hpp   # FSM para parseo
│   │   ├── Response.cpp/hpp        # Generador de respuesta
│   │   ├── HttpUtils.cpp/hpp       # Helpers HTTP
│   │   └── StatusCodes.cpp/hpp     # Códigos HTTP + mensajes
│   ├── handlers/
│   │   ├── Handler.cpp/hpp         # Interfaz base
│   │   ├── StaticHandler.cpp/hpp   # Archivos estáticos
│   │   ├── AutoIndexHandler.cpp/hpp# Directory listing
│   │   ├── CgiHandler.cpp/hpp      # CGI execution
│   │   ├── UploadHandler.cpp/hpp   # POST file upload
│   │   ├── DeleteHandler.cpp/hpp   # DELETE method
│   │   └── RedirectHandler.cpp/hpp # Redirecciones
│   └── utils/
│       ├── Logger.cpp/hpp          # Logging
│       ├── FileUtils.cpp/hpp       # File operations
│       ├── StringUtils.cpp/hpp     # String helpers
│       ├── MimeTypes.cpp/hpp       # Content-Type mapping
│       ├── PathUtils.cpp/hpp       # Path normalization
│       └── TimeoutManager.cpp/hpp  # Timeout handling
├── conf/
│   ├── default.conf                # Configuración por defecto
│   └── example.conf                # Ejemplo avanzado
├── www/
│   ├── index.html
│   ├── 404.html
│   ├── 500.html
│   └── [static files]
└── tests/
    ├── test_config.cpp
    ├── test_parser.cpp
    ├── test_handlers.cpp
    └── [test files]
```

---

## 🔑 COMPONENTES CLAVE

### 1️⃣ CONFIG READER
**De dónde sacamos qué:**
- **Webserver-2 (TOML)**: Idea buena pero el subject pide formato libre
- **Webserver-4**: Estructura jerárquica excelente (Server > Location)
- **Webserver-5**: Herencia de Context (muy elegante)

**Nuestro enfoque:**
```cpp
class Context {
    // Propiedades comunes a Server y Location
    std::string root;
    std::vector<std::string> index_files;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    // ...
};

class Location : public Context {
    std::string path;
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::string cgi_path;
    std::string redirect;
    // ...
};

class Server : public Context {
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::vector<Location> locations;
    // ...
};
```

**Parsing:**
- Leer línea por línea
- Detectar bloques `server { ... }` y `location ... { ... }`
- Validar directivas
- Herencia: Location obtiene propiedades de Server si no están definidas

---

### 2️⃣ EVENT LOOP (Multiplexación)
**De dónde sacamos qué:**
- **Webserver-1**: select() simple y efectivo
- **Webserver-2**: poll() + separación clara de FDs
- **Webserver-5**: epoll() para mejor rendimiento

**Nuestro enfoque:**
```cpp
class EventLoop {
    // Usar poll() como estándar (más portable que epoll, más moderno que select)
    std::vector<pollfd> pollfds;
    
    void addServerSocket(int fd);      // Añade socket de escucha
    void addClientSocket(int fd);      // Añade socket cliente
    void removeSocket(int fd);         // Elimina socket
    
    void run();                        // Main event loop
    void handleNewConnection();
    void handleClientData();
    void handleClientWrite();
};
```

**Características clave:**
- ✅ Non-blocking FDs (O_NONBLOCK)
- ✅ Timeouts para conexiones lentas
- ✅ Separación read/write
- ✅ Nunca bloquea el servidor

---

### 3️⃣ REQUEST PARSER (FSM)
**De dónde sacamos qué:**
- **Webserver-1**: Excelente máquina de estados
- **Webserver-5**: Excepciones para errores

**Estados del FSM:**
```
REQUEST_LINE -> HEADERS -> BODY -> COMPLETE
     ↑            ↑          ↑
     └── Si hay error → INVALID
```

**Nuestro RequestParser:**
```cpp
class RequestParser {
private:
    enum State {
        STATE_REQUEST_LINE,
        STATE_HEADERS,
        STATE_BODY,
        STATE_COMPLETE,
        STATE_ERROR
    };
    
    State current_state;
    std::string buffer;
    Request request;
    
public:
    ParseStatus parse(const std::string& chunk);
    // Retorna: INCOMPLETE, COMPLETE, ERROR
};
```

**Ventajas:**
- Maneja peticiones fragmentadas
- Soporta chunked encoding
- Rápido y eficiente
- Fácil de debuggear

---

### 4️⃣ RESPONSE GENERATOR
**De dónde sacamos qué:**
- **Webserver-4**: Dispatcher con función pointers o map
- **Webserver-5**: Herencia de Context + Longest Prefix Match

**Algoritmo de routing:**
1. Encuentra el `Server` que coincida (por puerto + server_name)
2. Encuentra el `Location` con **longest prefix match** (más específico gana)
3. Aplica permissiones (allowed_methods)
4. Maneja redirecciones
5. Despacha a handler específico

**Handler dispatch:**
```cpp
class Response {
    Handler* selectHandler(const Request& req, 
                           const Location& loc);
    
    // Si CGI → CgiHandler
    // Si es archivo → StaticHandler
    // Si es directorio → AutoIndexHandler o index file
    // Si POST → UploadHandler
    // Si DELETE → DeleteHandler
    // Si redirect → RedirectHandler
};
```

---

### 5️⃣ CGI EXECUTION
**De dónde sacamos qué:**
- **Webserver-1**: dup2 + archivos temporales
- **Webserver-4**: Variables de entorno bien preparadas
- **Webserver-5**: Manejo de chunked + pipes

**Nuestro enfoque:**
```cpp
class CgiHandler : public Handler {
    Response handle(const Request& req, const Location& loc);
    
    // 1. Preparar variables de entorno (CGI spec)
    // 2. Crear pipes para stdin/stdout
    // 3. fork()
    // 4. execve() en el hijo
    // 5. Esperar en el padre (sin bloquear el loop)
    // 6. Retornar respuesta al cliente
};
```

**Variables CGI que SON OBLIGATORIAS:**
- `REQUEST_METHOD`, `QUERY_STRING`
- `CONTENT_LENGTH`, `CONTENT_TYPE`
- `PATH_INFO`, `SCRIPT_NAME`
- `SERVER_NAME`, `SERVER_PORT`
- `SERVER_PROTOCOL`

---

## 🎯 DECISIONES ARQUITECTÓNICAS

### ✅ QUÉ USAR (Lo Mejor de Cada Proyecto)

| Aspecto | Decisión | Por Qué |
|---------|----------|--------|
| **Multiplexación** | poll() | Buen balance portabilidad/rendimiento |
| **Request Parser** | FSM (Webserver-1) | Robusto, eficiente, maneja chunked |
| **Config System** | Herencia Context (Webserver-5) | Elegante, DRY, fácil mantener |
| **Virtual Hosting** | Server + Location (Webserver-4) | Claro, escalable, como Nginx |
| **Routing** | Longest Prefix Match (Webserver-5) | Intuitivo, eficiente |
| **Error Handling** | Excepciones (Webserver-5) | Código limpio, fácil de seguir |
| **CGI** | Pipes + Variables entorno (Webserver-4) | Estándar CGI, bien probado |
| **Static Files** | Lectura directa (Webserver-1) | Simple, rápido |
| **Autoindex** | HTML dinámico (Webserver-4) | Buen UX, código simple |

---

## 🔄 CICLO DE VIDA COMPLETO DE UNA PETICIÓN

### Paso a Paso:

1. **ACCEPT** (Event Loop)
   - Nuevo cliente se conecta
   - Crear `ClientConnection`
   - Añadir a pollfd

2. **RECEIVE** (Event Loop)
   - poll detecta datos listos
   - recv() en socket
   - Pasar a RequestParser

3. **PARSE** (RequestParser - FSM)
   - Procesar byte por byte
   - Construir Request object
   - Validar
   - Si completo → ParseStatus::COMPLETE

4. **ROUTE** (Response)
   - Encontrar Server (puerto + Host header)
   - Encontrar Location (longest prefix match)
   - Validar método permitido
   - Aplicar permisos

5. **HANDLE** (Handler específico)
   - Si es CGI → CgiHandler (fork + execve)
   - Si es archivo → StaticHandler (leer disco)
   - Si es directorio → AutoIndexHandler (generar HTML)
   - Si POST → UploadHandler (guardar archivo)
   - Si DELETE → DeleteHandler (eliminar archivo)
   - Si redirect → RedirectHandler (enviar 301/302)

6. **SEND** (Event Loop - Non-blocking)
   - poll detecta socket listo para escribir
   - send() en trozos pequeños
   - No bloquear si falla parcial
   - Cuando termina → cerrar o keep-alive

7. **CLEANUP**
   - Remover socket del pollfd
   - Liberar memoria
   - Log de request

---

## 🚨 ERRORES A EVITAR (Visto en análisis)

### ❌ TRAMPAS COMUNES

1. **Bloqueo en CGI**
   - ❌ `waitpid()` bloqueante en el loop principal
   - ✅ Usar señales o epoll con el PID del child

2. **Buffer overflow en parsing**
   - ❌ Asumir petición entera en primer recv()
   - ✅ FSM que maneja fragmentación

3. **Fugas de memoria**
   - ❌ No cerrar file descriptors en errores
   - ✅ RAII patterns (constructores/destructores)

4. **Directory traversal**
   - ❌ Permitir `../../../etc/passwd`
   - ✅ `normalizePath()` que elimina `..` y `.`

5. **Errno check prohibido**
   - ❌ Revisar `errno` después de read/write
   - ✅ Revisar valor devuelto (-1) y usar non-blocking

6. **Headers malformados**
   - ❌ Asumir headers siempre presentes
   - ✅ Validar y tener defaults (Host, Content-Length)

7. **Chunked encoding ignorado**
   - ❌ No descodificar chunks
   - ✅ Parser debe entender formato chunked

---

## 📝 TABLA COMPARATIVA DE LOS 5 PROYECTOS

```
┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┐
│ Aspecto      │ Webserver-1  │ Webserver-2  │ Webserver-4  │ Webserver-5  │
├──────────────┼──────────────┼──────────────┼──────────────┼──────────────┤
│ Multiplex    │ select()     │ poll()       │ select()     │ epoll()      │
│ Parser FSM   │ ⭐⭐⭐      │ ⭐⭐       │ ⭐⭐       │ ⭐⭐       │
│ Config       │ simple       │ TOML         │ custom       │ herencia OK  │
│ Routing      │ básico       │ map          │ avanzado     │ prefix match │
│ CGI          │ dup2 OK      │ pipes        │ files        │ pipes OK     │
│ Error Pages  │ ⭐⭐⭐      │ ⭐⭐⭐      │ ⭐⭐⭐      │ ⭐⭐       │
│ Código limpio│ ⭐⭐       │ ⭐⭐⭐      │ ⭐⭐⭐      │ ⭐⭐⭐⭐    │
│ Escala       │ pequeño      │ medio        │ grande       │ muy grande   │
└──────────────┴──────────────┴──────────────┴──────────────┴──────────────┘
```

---

## ✅ CHECKLIST DE IMPLEMENTACIÓN

### FASE 1: FUNDAMENTOS (Semana 1)
- [ ] Makefile compilando C++98
- [ ] main.cpp con argument parsing
- [ ] Config reader básico (archivo .conf)
- [ ] ServerSocket::bind() y listen()
- [ ] Socket non-blocking (fcntl)

### FASE 2: MULTIPLEXACIÓN (Semana 1-2)
- [ ] EventLoop con poll()
- [ ] Accept nuevas conexiones
- [ ] Registrar/desregistrar sockets
- [ ] Manejo básico de entrada

### FASE 3: PARSING HTTP (Semana 2-3)
- [ ] Request clase
- [ ] RequestParser FSM completo
- [ ] Estados: REQUEST_LINE, HEADERS, BODY
- [ ] Manejo de chunked encoding
- [ ] Timeouts

### FASE 4: RESPONSE & ROUTING (Semana 3-4)
- [ ] Response generator
- [ ] Server + Location matching
- [ ] Longest prefix match
- [ ] Allowed methods check

### FASE 5: HANDLERS (Semana 4-5)
- [ ] StaticHandler (archivos estáticos)
- [ ] AutoIndexHandler (directory listing)
- [ ] UploadHandler (POST)
- [ ] DeleteHandler (DELETE)
- [ ] CgiHandler (fork + execve)
- [ ] RedirectHandler

### FASE 6: POLISH & TESTING (Semana 5-6)
- [ ] Error pages personalizadas
- [ ] MIME types correctos
- [ ] Timeouts de conexión
- [ ] Memory leaks
- [ ] Stress tests
- [ ] Browser testing

---

## 🧪 TESTING STRATEGY

```bash
# Test básico
curl http://localhost:8080/

# Test con headers
curl -H "Host: example.com" http://localhost:8080/

# Test POST
curl -X POST -d "data=test" http://localhost:8080/upload

# Test DELETE
curl -X DELETE http://localhost:8080/file.txt

# Test CGI
curl http://localhost:8080/script.php

# Stress test
ab -n 1000 -c 10 http://localhost:8080/

# Telnet (debugging)
telnet localhost 8080
GET / HTTP/1.1
Host: localhost

# Python test script (rápido de escribir)
# Ver archivo test_server.py
```

---

## 🎓 ESTRUCTURA DE ARCHIVOS RECOMENDADA PARA CADA CLASE

### Config.hpp
```cpp
class Server : public Context {
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::vector<Location> locations;
    
    Location* findLocation(const std::string& path);
};
```

### RequestParser.hpp
```cpp
enum ParseStatus { INCOMPLETE, COMPLETE, ERROR };

class RequestParser {
    ParseStatus parse(const std::string& chunk);
    Request getRequest() const;
    std::string getError() const;
};
```

### EventLoop.hpp
```cpp
class EventLoop {
    std::vector<pollfd> pollfds;
    std::map<int, ClientConnection*> connections;
    
    void run();
    void handleEvents();
};
```

---

## 📊 MÉTRICAS DE ÉXITO

✅ **Tu servidor debe:**
- Compilar sin warnings en C++98
- Soportar 100+ conexiones simultáneas
- No tener memory leaks
- Servir archivos estáticos en <50ms
- Ejecutar CGI sin bloquear
- Manejar URLs malformadas sin crash
- Responder a browser real
- Pasar stress tests
- Tener default error pages
- Soportar múltiples puertos

---

## 🚀 PRÓXIMOS PASOS

1. **Crea la estructura de carpetas**
2. **Empieza por Config + main()**
3. **Luego EventLoop + sockets**
4. **Después RequestParser (lo más importante)**
5. **Handlers en paralelo**
6. **Testing desde el día 1**

¡Adelante! 💪

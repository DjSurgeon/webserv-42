# 📊 WEBSERV - RESUMEN EJECUTIVO & ROADMAP

> Tu plan de ataque para crear el mejor webserv usando lo mejor de 5 proyectos

---

## 🎯 OBJETIVO

Crear un servidor HTTP **C++98** no bloqueante que:
- ✅ Compile sin warnings
- ✅ Maneje 100+ conexiones simultáneamente
- ✅ Sirva archivos estáticos
- ✅ Ejecute scripts CGI (PHP, Python)
- ✅ Soporte subidas de archivos (POST)
- ✅ Soporte eliminación de archivos (DELETE)
- ✅ Tenga páginas de error personalizadas
- ✅ Sea robusto (sin memory leaks, sin crashes)

---

## 🏆 LO MEJOR DE CADA PROYECTO

```
┌─────────────────────────────────────────────────────────────┐
│ WEBSERVER-1 (FuryWeb)                                       │
├─────────────────────────────────────────────────────────────┤
│ ⭐⭐⭐ RequestParser FSM (máquina de estados)              │
│ ⭐⭐⭐ select() + non-blocking I/O                         │
│ ⭐⭐⭐ Normalización de rutas (evita traversal)           │
│ ⭐⭐ Estructura básica pero sólida                         │
│ → USAR ESTO: El parser y la base del I/O                   │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ WEBSERVER-2 (WebservAchraf)                                 │
├─────────────────────────────────────────────────────────────┤
│ ⭐⭐⭐ poll() multiplexing (mejor que select)             │
│ ⭐⭐⭐ Arquitectura jerárquica (Server/Location)          │
│ ⭐⭐⭐ Manejo de errores robusto                          │
│ ⭐⭐ Config TOML (bonito pero no obligatorio)            │
│ → USAR ESTO: poll() + estructura jerárquica               │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ WEBSERVER-3                                                 │
├─────────────────────────────────────────────────────────────┤
│ ⭐⭐⭐ Validación robusta de inputs                        │
│ ⭐⭐ Error handling completo                               │
│ ⭐⭐ Testing approach claro                                │
│ → USAR ESTO: Filosofía de validación + testing             │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ WEBSERVER-4 (Cluster)                                       │
├─────────────────────────────────────────────────────────────┤
│ ⭐⭐⭐ CGI con fork() + execve() (bien implementado)      │
│ ⭐⭐⭐ RequestConfig dinámico                             │
│ ⭐⭐⭐ Separación clara de responsabilidades             │
│ ⭐⭐⭐ Dispatcher con function pointers                   │
│ → USAR ESTO: CGI execution + arquitectura modular          │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ WEBSERVER-5 (epoll)                                         │
├─────────────────────────────────────────────────────────────┤
│ ⭐⭐⭐⭐ Herencia de Context (elegante)                    │
│ ⭐⭐⭐⭐ Longest Prefix Match routing                      │
│ ⭐⭐⭐⭐ Excepciones para error handling                   │
│ ⭐⭐⭐ epoll() (aunque usaremos poll)                      │
│ → USAR ESTO: Context + routing + excepciones               │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎨 ARQUITECTURA FINAL (LO MEJOR DE TODO)

```
┌─────────────────────────────────────────┐
│         main() + ConfigReader            │
│     (Webserver-4 + Webserver-5)          │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│        EventLoop (poll)                  │
│         (Webserver-2)                    │
└────────────────┬────────────────────────┘
                 │
        ┌────────┴────────┐
        ▼                 ▼
    NEW CONN        CLIENT DATA
        │                │
        ▼                ▼
   ┌──────────┐   ┌──────────────────┐
   │ accept() │   │ RequestParser    │
   │          │   │ (FSM)            │
   │          │   │ (Webserver-1)    │
   └────┬─────┘   └────────┬─────────┘
        │                  │
        └──────────┬───────┘
                   ▼
        ┌────────────────────┐
        │ Response Generator │
        │ - Routing          │
        │ - Handler Select   │
        │ (Webserver-4/5)    │
        └────────────────────┘
              │
        ┌─────┴─────┬──────────┬──────────┐
        ▼           ▼          ▼          ▼
    Static       AutoIdx      CGI      Upload
    Handler      Handler     Handler   Handler
```

---

## 📋 TABLA DECISIONES ARQUITECTÓNICAS

| Decisión | Opción Elegida | De Quién | Justificación |
|----------|---------------|---------|----|
| **Multiplexación** | poll() | Webserver-2 | Buen balance, portabilidad |
| **Request Parser** | FSM (Máquina de Estados) | Webserver-1 | Robusto, maneja chunked |
| **Config System** | Herencia Context | Webserver-5 | Elegante, escalable, DRY |
| **Virtual Hosting** | Server + Location | Webserver-2/4/5 | Claro, como Nginx |
| **Routing** | Longest Prefix Match | Webserver-5 | Intuitivo, eficiente |
| **Error Handling** | Excepciones | Webserver-5 | Código limpio |
| **CGI Execution** | fork + execve | Webserver-4 | Estándar CGI, bien probado |
| **Static Files** | Lectura directa | Webserver-1 | Simple, rápido |
| **Autoindex** | HTML dinámico | Webserver-4 | Buen UX |
| **Path Safety** | normalizePath() | Webserver-1 | Evita traversal attacks |

---

## 🚀 ROADMAP DE IMPLEMENTACIÓN (6 SEMANAS)

### SEMANA 1: FUNDAMENTOS

**Fase 1.1 - Setup (Lunes)**
- [ ] Crear estructura de carpetas
- [ ] Makefile compilando C++98
- [ ] main.cpp con argument parsing (argc/argv)
- [ ] Logger básico para debugging
- **Tiempo: 2-3 horas**

**Fase 1.2 - Config (Martes-Miércoles)**
- [ ] Context.cpp/hpp (clase base)
- [ ] Server.cpp/hpp (hereda de Context)
- [ ] Location.cpp/hpp (hereda de Context)
- [ ] ConfigReader.cpp/hpp (parsea archivo .conf)
- [ ] Tests manuales: Leer config, printear
- **Tiempo: 6-8 horas**

**Fase 1.3 - Networking Base (Jueves-Viernes)**
- [ ] ServerSocket.cpp/hpp (bind + listen)
- [ ] SocketUtils (fcntl non-blocking, htons, etc)
- [ ] Test: Escuchar en puerto 8080
- [ ] Test: telnet localhost 8080 (conectar)
- **Tiempo: 4-5 horas**

---

### SEMANA 2: MULTIPLEXACIÓN

**Fase 2.1 - EventLoop (Lunes-Martes)**
- [ ] EventLoop.cpp/hpp con poll()
- [ ] ClientConnection.cpp/hpp
- [ ] Aceptar múltiples clientes
- [ ] Registrar/desregistrar sockets
- **Tiempo: 6-8 horas**

**Fase 2.2 - Request Parser Base (Miércoles-Viernes)**
- [ ] Request.cpp/hpp (estructura)
- [ ] RequestParser.cpp/hpp (estados: REQUEST_LINE, HEADERS)
- [ ] Tests: Parsear GET / HTTP/1.1
- [ ] Tests: Parsear Headers correctamente
- **Tiempo: 8-10 horas**

---

### SEMANA 3: PARSING HTTP COMPLETO

**Fase 3.1 - RequestParser Completo (Lunes-Martes)**
- [ ] Estado BODY (non-blocking)
- [ ] Soporte Content-Length
- [ ] Soporte chunked encoding
- [ ] Validaciones (URI length, method, etc)
- [ ] Error handling robustamente
- **Tiempo: 8-10 horas**

**Fase 3.2 - Response Base (Miércoles-Viernes)**
- [ ] Response.cpp/hpp (estructura)
- [ ] StatusCodes.cpp/hpp (200, 404, 405, etc)
- [ ] HttpUtils.cpp/hpp (formatear respuestas)
- [ ] MimeTypes.cpp/hpp (Content-Type mapping)
- [ ] Enviar respuesta HTTP válida
- **Tiempo: 6-8 horas**

---

### SEMANA 4: ROUTING & HANDLERS

**Fase 4.1 - Routing Jerárquico (Lunes-Miércoles)**
- [ ] Response::findServer() (por puerto + Host)
- [ ] Location::findLocation() (longest prefix match)
- [ ] Validar método permitido
- [ ] Tests: Múltiples servers, locations
- **Tiempo: 6-8 horas**

**Fase 4.2 - Static File Handler (Miércoles-Jueves)**
- [ ] Handler.cpp/hpp (interfaz base)
- [ ] StaticHandler.cpp/hpp
- [ ] Leer archivos del disco
- [ ] Servir 404, 403, etc
- [ ] Tests: GET archivo.html
- **Tiempo: 4-5 horas**

**Fase 4.3 - Directory Listing (Viernes)**
- [ ] AutoIndexHandler.cpp/hpp
- [ ] Generar HTML dinámico
- [ ] PathUtils::normalizePath() (evita traversal)
- [ ] Tests: GET /directorio/
- **Tiempo: 4-5 horas**

---

### SEMANA 5: CGI & UPLOAD/DELETE

**Fase 5.1 - Upload Handler (Lunes-Martes)**
- [ ] UploadHandler.cpp/hpp
- [ ] Parsear multipart/form-data
- [ ] Guardar archivo a disco
- [ ] Validar tamaño
- [ ] Tests: curl -F "file=@test.txt"
- **Tiempo: 6-8 horas**

**Fase 5.2 - Delete Handler (Miércoles)**
- [ ] DeleteHandler.cpp/hpp
- [ ] Eliminar archivo si existe
- [ ] Validar permisos
- [ ] Tests: curl -X DELETE
- **Tiempo: 2-3 horas**

**Fase 5.3 - CGI Handler (Jueves-Viernes)**
- [ ] CgiHandler.cpp/hpp
- [ ] fork() + execve()
- [ ] Preparar variables de entorno CGI
- [ ] Redirección de stdin/stdout (dup2)
- [ ] Lectura de salida CGI
- [ ] Tests: GET script.php, GET script.py
- **Tiempo: 8-10 horas**

---

### SEMANA 6: POLISH & TESTING

**Fase 6.1 - Error Pages Personalizadas (Lunes)**
- [ ] RedirectHandler.cpp/hpp
- [ ] Cargar error pages desde config
- [ ] Fallback a error pages por defecto
- [ ] Tests: 404, 500, etc
- **Tiempo: 3-4 horas**

**Fase 6.2 - Timeouts & Keep-Alive (Martes)**
- [ ] TimeoutManager.cpp/hpp
- [ ] Detectar clientes muertos
- [ ] Cerrar conexiones inactivas
- [ ] Soporte HTTP Keep-Alive
- **Tiempo: 4-5 horas**

**Fase 6.3 - Testing & Debugging (Miércoles-Viernes)**
- [ ] Browser testing (Firefox, Chrome)
- [ ] Stress tests (ab, hey, wrk)
- [ ] Memory leak check (valgrind)
- [ ] Pruebas edge cases
- [ ] Documentación y comments
- **Tiempo: 10-12 horas**

---

## 📝 CHECKLIST FINAL

### Funcionalidades Básicas
- [ ] Compilar sin warnings
- [ ] Config file (puerto, host, root, index, etc)
- [ ] Socket non-blocking
- [ ] poll() multiplexing
- [ ] GET files estáticos
- [ ] Múltiples servers (ports)
- [ ] Múltiples locations por server
- [ ] Método GET, POST, DELETE
- [ ] Headers correctos (CRLF)
- [ ] Content-Length exacto

### Seguridad & Robustez
- [ ] No memory leaks
- [ ] Validación de inputs
- [ ] Directory traversal protection
- [ ] Máximo size de request
- [ ] Timeout de conexiones
- [ ] Error pages personalizadas
- [ ] Nunca crash (ni con bad input)
- [ ] Non-blocking 100%

### CGI & Dinámico
- [ ] Ejecutar scripts .php
- [ ] Ejecutar scripts .py
- [ ] Variables de entorno CGI
- [ ] Parsing de multipart (upload)
- [ ] Redirecciones (301, 302)
- [ ] Autoindex (directory listing)

### Testing
- [ ] Funciona con browser real
- [ ] Stress test 1000 requests
- [ ] Conexiones simultáneas
- [ ] Valgrind sin leaks
- [ ] curl tests
- [ ] telnet manual
- [ ] Casos edge

---

## 💡 DECISIONES IMPORTANTES

### ✅ HACER

1. **Empezar por ConfigReader**
   - Define la estructura desde el inicio
   - Todo lo demás depende de esto

2. **EventLoop antes de handlers**
   - El "corazón" del servidor
   - Una vez que funciona, todo es más fácil

3. **RequestParser es la joya**
   - Párate el tiempo que sea necesario
   - Es lo más importante

4. **Tests desde el día 1**
   - Cada componente testeado inmediatamente
   - Debugging más fácil

5. **Usar make re durante desarrollo**
   - Compilación limpia frecuente
   - Detecta problemas temprano

### ❌ NO HACER

1. **Usar threads** - Poll es suficiente
2. **Boost libraries** - Subject lo prohíbe
3. **Regex para parsing** - FSM es más rápido
4. **system()** para CGI - fork + execve
5. **Globals** - RAII y clases
6. **String.h en lugar de cstring** - C++98
7. **Asumir Headers siempre presentes**
8. **Revisar errno** - Prohibido

---

## 🧪 COMANDO CLAVE PARA TESTING

```bash
# En 5 terminales simultáneas:

# Terminal 1: Tu servidor
./webserv conf/default.conf

# Terminal 2: Tests básicos
curl http://localhost:8080/
curl -X POST -d "test=data" http://localhost:8080/upload
curl -X DELETE http://localhost:8080/file.txt

# Terminal 3: Stress test
ab -n 1000 -c 100 http://localhost:8080/

# Terminal 4: Memory check
valgrind ./webserv conf/default.conf

# Terminal 5: Manual telnet
telnet localhost 8080
# GET / HTTP/1.1
# Host: localhost
# [enter vacío]
```

---

## 📚 ESTRUCTURA TÍPICA DE CARPETAS A FIN DE PROYECTO

```
webserv/
├── Makefile
├── src/
│   ├── main.cpp
│   ├── config/          (4 archivos)
│   ├── network/         (3 archivos)
│   ├── http/            (4 archivos)
│   ├── handlers/        (7 archivos)
│   └── utils/           (6 archivos)
├── conf/
│   ├── default.conf
│   └── example.conf
├── www/                 (archivos estáticos)
│   ├── index.html
│   ├── 404.html
│   └── [más archivos]
└── tests/               (scripts de testing)
```

**Total: ~30-35 archivos .cpp/hpp**

---

## 🎓 REQUISITOS MÍNIMOS QUE DEBE CUMPLIR

Según el subject:

- ✅ C++98 (compilar con -std=c++98)
- ✅ Non-blocking I/O (fcntl O_NONBLOCK)
- ✅ poll (o select/epoll/kqueue equivalente)
- ✅ Un socket servidor por puerto
- ✅ Múltiples clients simultáneos
- ✅ GET, POST, DELETE methods
- ✅ Config file (con argumentos)
- ✅ Virtual servers (server_name)
- ✅ Multiple locations por server
- ✅ Default error pages
- ✅ Static files
- ✅ File uploads
- ✅ CGI execution
- ✅ Directory listing (opcional pero fácil)
- ✅ Redirects

---

## 🏁 DIFERENCIAL (Para 100/100)

- Código muy limpio (RAII, sin globals)
- Zero memory leaks
- Manejo excelente de errores
- Stress test sin problemas
- Browser compatibility
- Edge cases contemplados
- Keep-alive soportado
- Chunked encoding soportado
- Comments en código
- Makefile profesional

---

## 🎉 ¡LISTO PARA EMPEZAR!

Has leído:
1. ✅ **webserv_master_plan.md** - Arquitectura completa
2. ✅ **webserv_code_examples.md** - Código real funcionando
3. ✅ **webserv_best_practices.md** - Errores a evitar
4. ✅ **webserv_makefile_and_config.md** - Makefile + config
5. ✅ **Este documento** - Resumen ejecutivo

## ⚡ PRÓXIMOS PASOS

1. Crea la carpeta `webserv/src`
2. Copia el Makefile
3. Empieza por `main.cpp` y `ConfigReader`
4. Haz un test: `make && ./webserv conf/default.conf`
5. Luego EventLoop
6. Luego RequestParser

**Tiempo total estimado: 40-50 horas**

**Dificultad: MEDIA-ALTA** (pero totalmente factible)

**Satisfacción al final: ⭐⭐⭐⭐⭐**

¡A por ello! 💪🚀

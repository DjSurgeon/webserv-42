# ⚡ WEBSERV - QUICK START (30 MINUTOS)

> Comienza a codificar en los próximos 30 minutos

---

## 🎯 OBJETIVO DE ESTA SESIÓN

Tener un servidor compilando y escuchando en puerto 8080

```
Tu servidor →  ./webserv
               → "Server listening on 0.0.0.0:8080"
               → curl http://localhost:8080/
               → "Hello from webserv!" ✅
```

---

## 📂 PASO 1: CREAR ESTRUCTURA (2 minutos)

```bash
mkdir -p webserv
cd webserv
mkdir -p src/{config,network,http,handlers,utils} conf www tests
```

Resultado:
```
webserv/
├── src/
│   ├── config/
│   ├── network/
│   ├── http/
│   ├── handlers/
│   ├── utils/
│   └── main.cpp (crear)
├── conf/
│   └── default.conf (crear)
└── www/
    └── index.html (crear)
```

---

## 📋 PASO 2: CREAR MAKEFILE (3 minutos)

Crea `webserv/Makefile`:

```makefile
NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
LDFLAGS =

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = src

SOURCES = $(SRC_DIR)/main.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $(NAME) $(OBJECTS) $(LDFLAGS)
	@echo "✅ $(NAME) compilado"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
```

---

## 🔧 PASO 3: CREAR main.cpp BÁSICO (5 minutos)

Crea `src/main.cpp`:

```cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    // 1. Parsing arguments
    std::string config_file = "conf/default.conf";
    if (argc > 1)
        config_file = argv[1];

    std::cout << "Using config: " << config_file << std::endl;

    // 2. Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // 3. Set socket option (reusar puerto)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, 
                   &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << std::endl;
        return 1;
    }

    // 4. Bind
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // 5. Listen
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    // 6. Non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    std::cout << "✅ Server listening on 0.0.0.0:8080" << std::endl;
    std::cout << "Test with: curl http://localhost:8080/" << std::endl;

    // 7. Simple loop (TODO: Replace with poll)
    sleep(60);  // Escuchar 60 segundos

    close(server_fd);
    return 0;
}
```

---

## ⚙️ PASO 4: CREAR CONFIG MÍNIMO (2 minutos)

Crea `conf/default.conf`:

```
server {
    listen 8080;
    host 0.0.0.0;
    server_name localhost;
    
    root ./www;
    index index.html;
    
    client_max_body_size 1000000;
    autoindex on;
    
    location / {
        allowed_methods GET;
        root ./www;
        index index.html;
    }
}
```

---

## 📄 PASO 5: CREAR index.html (1 minuto)

Crea `www/index.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Webserv</title>
</head>
<body>
    <h1>Welcome to Webserv!</h1>
    <p>Your HTTP server is working!</p>
</body>
</html>
```

---

## 🏗️ PASO 6: COMPILAR Y PROBAR (3 minutos)

```bash
# Compilar
make clean
make

# Ejecutar
./webserv conf/default.conf

# En otra terminal, probar:
curl http://localhost:8080/
# → Debería ver el mensaje "Server listening..."
```

---

## ✅ SI LLEGASTE AQUÍ

Tienes:
- ✅ Estructura de carpetas
- ✅ Makefile funcional
- ✅ main.cpp compilando
- ✅ Socket escuchando
- ✅ Archivo config
- ✅ index.html

**TIEMPO TOTAL: ~15 minutos**

---

## 🚀 PRÓXIMOS PASOS (SEMANA 1)

### Día 2: Agregar poll()

Reemplaza el `sleep(60)` en main.cpp con:

```cpp
#include <poll.h>

// Después de listen()
pollfd pfd;
pfd.fd = server_fd;
pfd.events = POLLIN;

std::cout << "Waiting for connections (Ctrl+C to exit)..." << std::endl;

while (true) {
    int ready = poll(&pfd, 1, 1000);  // timeout 1 segundo
    
    if (ready < 0) {
        std::cerr << "poll error" << std::endl;
        break;
    }
    
    if (ready > 0 && pfd.revents & POLLIN) {
        // Cliente conectó
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            std::cerr << "accept error" << std::endl;
            continue;
        }
        
        std::cout << "✅ Client connected" << std::endl;
        close(client_fd);
    }
}
```

### Día 3: RequestParser

Crea `src/http/RequestParser.hpp` y `.cpp`

### Día 4: Response

Crea `src/http/Response.hpp` y `.cpp`

### Día 5: Config System

Implementa `src/config/ConfigReader.hpp`

---

## 🧪 MINI TESTS

```bash
# Test 1: ¿Se compila?
make
# → Debería decir "✅ webserv compilado"

# Test 2: ¿Escucha en puerto?
./webserv &
# → Debería decir "✅ Server listening on 0.0.0.0:8080"

# Test 3: ¿Puedes conectar?
netstat -tlnp | grep 8080
# → Debería mostrar LISTEN

# Test 4: ¿Acepta conexión?
timeout 1 nc -v localhost 8080
# → Debería conectar (aunque aún no responda HTTP)
```

---

## ⚠️ ERRORES COMUNES EN ESTE PUNTO

### Error: "Address already in use"
```bash
# Solución: El puerto sigue abierto del intento anterior
# Espera 30 segundos o mata el proceso
pkill -f webserv
sleep 5
make && ./webserv
```

### Error: "Permission denied" (puerto < 1024)
```bash
# Tu código usa puerto 8080, que es OK
# Si cambiaste a puerto 80, necesitas sudo (evita hacer esto)
```

### Error: "Bind failed"
Asegúrate de tener `SO_REUSEADDR` en `setsockopt()`

### Compilación lenta
Es normal en C++. Siguiente compilación será más rápida.

---

## 📝 CHECKLIST MINI

- [ ] Carpetas creadas
- [ ] Makefile existe
- [ ] main.cpp compila
- [ ] Configuración archivo existe
- [ ] index.html existe
- [ ] ./webserv ejecuta sin errores
- [ ] `curl localhost:8080` intenta conectar
- [ ] `Ctrl+C` detiene el servidor limpiamente

¿TODO ✅? **¡Felicidades! Ya tienes la base!**

---

## 🎯 META REALISTA

**Esta sesión:** Setup básico funcional
**Siguiente sesión:** Agregar poll() + aceptar múltiples clientes
**Esta semana:** Parsear request HTTP
**Siguiente semana:** Servir archivos estáticos
**Proyecto completo:** 6 semanas

---

## 📚 DOCUMENTOS RELACIONADOS

Después de esto, lee:
1. webserv_master_plan.md (para entender qué viene)
2. webserv_code_examples.md (para copiar estructura)
3. webserv_diagrams.md (para entender los flujos)

---

## 💡 TIP IMPORTANTE

**No copies el código**, entiéndelo:
- ¿Por qué O_NONBLOCK?
- ¿Por qué SO_REUSEADDR?
- ¿Qué hace fcntl()?
- ¿Para qué es poll()?

Cuando entendes, debugging es 10x más fácil.

---

## 🎉 ¡LO HICISTE!

Tienes un servidor HTTP básico compilando y escuchando.

Próximo: Hacer que responda con HTML.

**Tiempo para lo anterior: ~30 minutos**
**Tiempo para responder HTTP: ~2 horas más**

¡Vamos! 🚀

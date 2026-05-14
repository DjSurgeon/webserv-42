# ========================================================
# WEBSERV - MAKEFILE TEMPLATE
# ========================================================

NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
LDFLAGS = 

# Directorios
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = src

# Archivos fuente
SOURCES = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/config/Config.cpp \
	$(SRC_DIR)/config/ConfigReader.cpp \
	$(SRC_DIR)/config/Server.cpp \
	$(SRC_DIR)/config/Location.cpp \
	$(SRC_DIR)/config/Context.cpp \
	$(SRC_DIR)/network/EventLoop.cpp \
	$(SRC_DIR)/network/ServerSocket.cpp \
	$(SRC_DIR)/network/ClientConnection.cpp \
	$(SRC_DIR)/network/SocketUtils.cpp \
	$(SRC_DIR)/http/Request.cpp \
	$(SRC_DIR)/http/RequestParser.cpp \
	$(SRC_DIR)/http/Response.cpp \
	$(SRC_DIR)/http/HttpUtils.cpp \
	$(SRC_DIR)/http/StatusCodes.cpp \
	$(SRC_DIR)/handlers/Handler.cpp \
	$(SRC_DIR)/handlers/StaticHandler.cpp \
	$(SRC_DIR)/handlers/AutoIndexHandler.cpp \
	$(SRC_DIR)/handlers/CgiHandler.cpp \
	$(SRC_DIR)/handlers/UploadHandler.cpp \
	$(SRC_DIR)/handlers/DeleteHandler.cpp \
	$(SRC_DIR)/handlers/RedirectHandler.cpp \
	$(SRC_DIR)/utils/Logger.cpp \
	$(SRC_DIR)/utils/FileUtils.cpp \
	$(SRC_DIR)/utils/StringUtils.cpp \
	$(SRC_DIR)/utils/MimeTypes.cpp \
	$(SRC_DIR)/utils/PathUtils.cpp \
	$(SRC_DIR)/utils/TimeoutManager.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
INCLUDES = -I$(INC_DIR)

# ========================================================
# TARGETS
# ========================================================

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(NAME) $(OBJECTS) $(LDFLAGS)
	@echo "✅ $(NAME) compilado correctamente"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo "🔨 Compilando $<"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "🗑️  Objetos eliminados"

fclean: clean
	@rm -f $(NAME)
	@echo "🗑️  Ejecutable eliminado"

re: fclean all

# ========================================================
# UTILIDADES
# ========================================================

run: all
	./$(NAME) conf/default.conf

debug: CXXFLAGS += -g -O0
debug: re

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME) conf/default.conf

test: all
	./tests/test_parser.sh
	./tests/test_config.sh
	./tests/test_handlers.sh

.PHONY: all clean fclean re run debug valgrind test

# ========================================================
# MACROS PARA DEBUGGING
# ========================================================
# Usa: make clean && make VERBOSE=1
ifdef VERBOSE
    CXXFLAGS += -DVERBOSE
endif

---

# ========================================================
# WEBSERV - ARCHIVO DE CONFIGURACIÓN DE EJEMPLO
# ========================================================

# Archivo: conf/default.conf

# ========================================================
# SERVIDOR 1 - Puerto 8080 (Default)
# ========================================================
server {
    listen 8080;
    host 0.0.0.0;
    server_name example.com www.example.com;

    root /var/www/html;
    index index.html index.htm;

    client_max_body_size 10000000;  # 10MB
    autoindex on;

    # Error pages personalizadas
    error_page 404 /404.html;
    error_page 500 /500.html;
    error_page 403 /403.html;

    # ========================================
    # Location: Archivos estáticos
    # ========================================
    location / {
        allowed_methods GET HEAD;
        root /var/www/html;
        index index.html;
        autoindex on;
    }

    # ========================================
    # Location: Subir archivos
    # ========================================
    location /upload {
        allowed_methods POST GET;
        root /var/www/html;
        upload_path /var/www/uploads;
        client_max_body_size 5000000;  # 5MB para uploads
    }

    # ========================================
    # Location: Eliminar archivos
    # ========================================
    location /delete {
        allowed_methods DELETE GET;
        root /var/www/html;
    }

    # ========================================
    # Location: Scripts CGI (.php)
    # ========================================
    location /cgi {
        allowed_methods GET POST;
        cgi_path /usr/bin/php-cgi;
        cgi_extension .php;
        root /var/www/cgi-bin;
    }

    # ========================================
    # Location: Scripts CGI (.py)
    # ========================================
    location /python {
        allowed_methods GET POST;
        cgi_path /usr/bin/python3;
        cgi_extension .py;
        root /var/www/scripts;
    }

    # ========================================
    # Location: Redirección
    # ========================================
    location /old-page {
        redirect /new-page;
        return 301;
    }
}

# ========================================================
# SERVIDOR 2 - Puerto 8081 (Alternativo)
# ========================================================
server {
    listen 8081;
    host 127.0.0.1;
    server_name localhost;

    root /var/www/localhost;
    index index.html;

    client_max_body_size 1000000;

    # Solo permite GET en este servidor
    location / {
        allowed_methods GET;
        root /var/www/localhost;
    }

    # Autoindex habilitado solo aquí
    location /files {
        root /var/www/shared;
        autoindex on;
    }
}

# ========================================================
# SERVIDOR 3 - Puerto 8082 (API)
# ========================================================
server {
    listen 8082;
    host 0.0.0.0;
    server_name api.example.com;

    root /var/www/api;

    client_max_body_size 1000000;

    # API endpoints
    location /api/users {
        allowed_methods GET POST DELETE;
        root /var/www/api;
    }

    location /api/products {
        allowed_methods GET;
        root /var/www/api;
    }
}

---

# ========================================================
# WEBSERV - ARCHIVO DE CONFIGURACIÓN AVANZADO
# (Comentarios explicativos)
# ========================================================

# Archivo: conf/advanced.conf

# ========================================================
# DIRECTIVAS GLOBALES (aplican a todos los servers)
# ========================================================
# No hay directivas globales en nuestro formato
# (Cada server define sus propias reglas)

# ========================================================
# SERVIDOR - Estructura General
# ========================================================
server {
    # 1. ESCUCHA
    listen 8080;              # Puerto (obligatorio)
    host 0.0.0.0;            # IP (0.0.0.0 = todas)

    # 2. VIRTUAL HOSTING
    server_name example.com www.example.com;  # Múltiples nombres

    # 3. RAÍZ DEL SITIO
    root /var/www/html;      # Directorio base

    # 4. ARCHIVOS POR DEFECTO
    index index.html index.htm index.php;  # En orden de preferencia

    # 5. LÍMITES
    client_max_body_size 1000000;  # Máximo tamaño de request (bytes)

    # 6. LISTADO DE DIRECTORIOS
    autoindex on;            # Mostrar contenido si no hay index

    # 7. PÁGINAS DE ERROR
    error_page 400 /400.html;
    error_page 401 /401.html;
    error_page 403 /403.html;
    error_page 404 /404.html;
    error_page 405 /405.html;
    error_page 413 /413.html;
    error_page 500 /500.html;
    error_page 502 /502.html;
    error_page 503 /503.html;

    # ========================================
    # LOCATIONS - Subrutas del servidor
    # ========================================

    # Location 1: Raíz (catch-all)
    location / {
        allowed_methods GET HEAD;
        root /var/www/html;
        index index.html;
        autoindex off;
    }

    # Location 2: Directorio protegido
    location /admin {
        allowed_methods GET HEAD;
        root /var/www/admin;
        autoindex off;
        # No se puede subir ni eliminar
    }

    # Location 3: Subir archivos
    location /upload {
        allowed_methods POST GET;
        root /var/www/html;
        upload_path /var/www/uploads;  # Dónde guardar
        client_max_body_size 5000000;   # Límite específico
    }

    # Location 4: Descargar/Eliminar
    location /files {
        allowed_methods GET DELETE;
        root /var/www/shared;
        autoindex on;
    }

    # Location 5: CGI (PHP)
    location /app {
        allowed_methods GET POST;
        root /var/www/app;
        cgi_path /usr/bin/php-cgi;
        cgi_extension .php;
        index index.php;
    }

    # Location 6: CGI (Python)
    location /scripts {
        allowed_methods GET POST;
        root /var/www/scripts;
        cgi_path /usr/bin/python3;
        cgi_extension .py;
    }

    # Location 7: Redirección
    location /legacy {
        redirect http://example.com/new;
        return 301;
    }

    # Location 8: Solo archivos estáticos
    location /assets {
        allowed_methods GET HEAD;
        root /var/www/assets;
        autoindex off;
    }
}

# ========================================================
# NOTAS SOBRE LA CONFIGURACIÓN
# ========================================================

# 1. LISTEN Y HOST
#    - listen 8080 → Escucha en puerto 8080
#    - host 0.0.0.0 → Aceptar de cualquier IP
#    - host 127.0.0.1 → Solo localhost
#    - host 192.168.1.1 → Solo esa IP

# 2. SERVER_NAME
#    - Sirve para virtual hosting
#    - Múltiples nombres permitidos
#    - El header "Host" del cliente determina cuál

# 3. ROOT
#    - En Server: raíz del sitio
#    - En Location: sobrescribe la del Server
#    - Todas las rutas son relativas a este

# 4. INDEX
#    - Archivos a buscar si pides directorio
#    - Se buscan en orden hasta encontrar uno

# 5. AUTOINDEX
#    - on: Generar HTML con lista de archivos
#    - off: Dar error 403 (Forbidden)

# 6. ALLOWED_METHODS
#    - GET: Descargar archivo
#    - POST: Subir archivo
#    - DELETE: Eliminar archivo
#    - HEAD: Igual que GET pero sin body

# 7. CGI_PATH y CGI_EXTENSION
#    - CGI_PATH: Dónde está el intérprete (php-cgi, python3)
#    - CGI_EXTENSION: Qué extensión activa el CGI (.php, .py)

# 8. UPLOAD_PATH
#    - Dónde guardar archivos subidos
#    - Debe ser un directorio que exista
#    - El servidor debe tener permisos de escritura

# 9. CLIENT_MAX_BODY_SIZE
#    - Límite de tamaño de requests
#    - En Server: aplica a todo
#    - En Location: puede sobrescribir

# 10. ERROR_PAGE
#     - Mapear códigos HTTP a archivos HTML
#     - Si no existe el archivo, mostrar error genérico

# 11. REDIRECT
#     - Redirigir a otra URL
#     - Típicamente con return 301 (movido permanentemente)
#     - O return 302 (movido temporalmente)

# ========================================================
# ESTRUCTURA DE DIRECTORIOS RECOMENDADA
# ========================================================

# /var/www/
# ├── html/                    # Raíz por defecto (location /)
# │   ├── index.html
# │   ├── 404.html
# │   ├── 500.html
# │   └── assets/              # Archivos estáticos
# │       ├── style.css
# │       └── script.js
# ├── uploads/                 # Archivos subidos (location /upload)
# ├── app/                     # CGI PHP (location /app)
# │   ├── index.php
# │   └── config.php
# ├── scripts/                 # CGI Python (location /scripts)
# │   ├── hello.py
# │   └── api.py
# ├── admin/                   # Área privada (location /admin)
# │   └── dashboard.html
# └── shared/                  # Descargas (location /files)
#     └── document.pdf

# ========================================================
# COMANDOS ÚTILES PARA TESTING
# ========================================================

# Servidor en escucha
# ./webserv conf/default.conf

# Test GET
# curl http://localhost:8080/

# Test GET con headers específicos
# curl -H "Host: example.com" http://localhost:8080/

# Test POST (subir)
# curl -X POST -d "data=test" http://localhost:8080/upload

# Test DELETE
# curl -X DELETE http://localhost:8080/files/document.txt

# Test CGI
# curl http://localhost:8080/app/index.php?name=John

# Test error pages
# curl http://localhost:8080/nonexistent  # 404
# curl -X INVALID http://localhost:8080/  # 405

# Test con cliente lento (stream)
# python3 -c "import socket; s=socket.socket(); s.connect(('localhost',8080)); \
# s.send(b'GET / HTTP/1.1\r\nHost: localhost\r\n'); \
# import time; time.sleep(2); s.send(b'\r\n'); s.recv(4096)"

# Stress test
# ab -n 1000 -c 100 http://localhost:8080/

# ========================================================

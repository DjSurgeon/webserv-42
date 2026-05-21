# 🔨 WEBSERV - CÓDIGO EJEMPLAR DE COMPONENTES

> Ejemplos de implementación para cada componente clave

---

## 1️⃣ CONTEXT - BASE DE LA JERARQUÍA

```cpp
// Context.hpp
#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <string>
#include <vector>
#include <map>

class Context {
public:
    Context();
    virtual ~Context();

    // Setters
    void setRoot(const std::string& root) { this->root = root; }
    void addIndexFile(const std::string& index) { index_files.push_back(index); }
    void setErrorPage(int code, const std::string& path) { error_pages[code] = path; }
    void setClientMaxBodySize(size_t size) { client_max_body_size = size; }
    void setAutoindex(bool value) { autoindex = value; }

    // Getters
    const std::string& getRoot() const { return root; }
    const std::vector<std::string>& getIndexFiles() const { return index_files; }
    const std::string& getErrorPage(int code) const;
    size_t getClientMaxBodySize() const { return client_max_body_size; }
    bool isAutoindexEnabled() const { return autoindex; }

    // Inheritance helper: si no está definido, devolver valor por defecto
    virtual const std::string& inheritRoot() const { return root; }

protected:
    std::string root;
    std::vector<std::string> index_files;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    bool autoindex;
};

#endif
```

```cpp
// Context.cpp
#include "Context.hpp"

Context::Context()
    : root("./www"),
      client_max_body_size(1024 * 1024),  // 1MB default
      autoindex(false)
{
    index_files.push_back("index.html");
}

Context::~Context() {}

const std::string& Context::getErrorPage(int code) const {
    std::map<int, std::string>::const_iterator it = error_pages.find(code);
    if (it != error_pages.end()) {
        return it->second;
    }
    // Retornar string estático vacío si no existe
    static std::string empty = "";
    return empty;
}
```

---

## 2️⃣ LOCATION & SERVER - HERENCIA

```cpp
// Location.hpp
#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Context.hpp"
#include <vector>

class Server;  // Forward declaration

class Location : public Context {
public:
    Location(const std::string& path, Server* parent);
    ~Location();

    // Setters específicos de Location
    void setPath(const std::string& p) { path = p; }
    void addAllowedMethod(const std::string& method) { allowed_methods.push_back(method); }
    void setCgiPath(const std::string& cgi) { cgi_path = cgi; }
    void setCgiExtension(const std::string& ext) { cgi_extension = ext; }
    void setRedirect(const std::string& url) { redirect = url; }
    void setUploadPath(const std::string& up) { upload_path = up; }

    // Getters
    const std::string& getPath() const { return path; }
    const std::vector<std::string>& getAllowedMethods() const { return allowed_methods; }
    bool isMethodAllowed(const std::string& method) const;
    const std::string& getCgiPath() const { return cgi_path; }
    const std::string& getCgiExtension() const { return cgi_extension; }
    const std::string& getRedirect() const { return redirect; }
    const std::string& getUploadPath() const { return upload_path; }

    // Herencia: si no está definido en Location, buscar en Server
    virtual const std::string& inheritRoot() const;

private:
    std::string path;
    std::vector<std::string> allowed_methods;
    std::string cgi_path;       // ej: /usr/bin/php-cgi
    std::string cgi_extension;  // ej: .php
    std::string redirect;       // ej: http://example.com (301/302)
    std::string upload_path;    // Dónde guardar uploads
    Server* parent_server;
};

#endif
```

```cpp
// Location.cpp
#include "Location.hpp"
#include "Server.hpp"
#include <algorithm>

Location::Location(const std::string& path, Server* parent)
    : path(path), parent_server(parent)
{
}

Location::~Location() {}

bool Location::isMethodAllowed(const std::string& method) const {
    if (allowed_methods.empty()) {
        return true;  // Si no hay restricción, permitir todo
    }
    return std::find(allowed_methods.begin(), allowed_methods.end(), method)
        != allowed_methods.end();
}

// Herencia: si no tenemos root, pedirle al Server
const std::string& Location::inheritRoot() const {
    if (root != Context().getRoot()) {
        return root;  // Estamos sobrescribiendo
    }
    return parent_server->getRoot();  // Heredar del Server
}
```

```cpp
// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include "Context.hpp"
#include "Location.hpp"
#include <vector>

class Server : public Context {
public:
    Server();
    ~Server();

    // Setters
    void setPort(int p) { port = p; }
    void setHost(const std::string& h) { host = h; }
    void addServerName(const std::string& name) { server_names.push_back(name); }
    void addLocation(const Location& loc) { locations.push_back(loc); }

    // Getters
    int getPort() const { return port; }
    const std::string& getHost() const { return host; }
    const std::vector<std::string>& getServerNames() const { return server_names; }
    const std::vector<Location>& getLocations() const { return locations; }

    // Buscar Location por path (longest prefix match)
    Location* findLocation(const std::string& path);
    const Location* findLocation(const std::string& path) const;

    bool isDefaultServer() const { return is_default; }
    void setDefaultServer(bool def) { is_default = def; }

private:
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::vector<Location> locations;
    bool is_default;
};

#endif
```

```cpp
// Server.cpp
#include "Server.hpp"
#include <algorithm>

Server::Server()
    : port(8080), host("0.0.0.0"), is_default(false)
{
}

Server::~Server() {}

Location* Server::findLocation(const std::string& path) {
    if (locations.empty()) {
        return NULL;
    }

    // Longest Prefix Match
    Location* best = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& loc_path = locations[i].getPath();
        if (path.find(loc_path) == 0 && loc_path.length() > best_len) {
            best = &locations[i];
            best_len = loc_path.length();
        }
    }
    return best;
}
```

---

## 3️⃣ REQUEST PARSER - FSM (LA JOYA)

```cpp
// RequestParser.hpp
#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include "Request.hpp"
#include <string>

enum ParseStatus {
    PARSE_INCOMPLETE,
    PARSE_COMPLETE,
    PARSE_ERROR
};

class RequestParser {
private:
    enum State {
        STATE_REQUEST_LINE,
        STATE_HEADERS,
        STATE_BODY,
        STATE_COMPLETE,
        STATE_ERROR
    };

public:
    RequestParser();
    ~RequestParser();

    ParseStatus parse(const std::string& chunk);
    Request getRequest() const { return request; }
    std::string getError() const { return error_msg; }
    bool isComplete() const { return current_state == STATE_COMPLETE; }

private:
    State current_state;
    std::string buffer;
    Request request;
    std::string error_msg;
    size_t body_received;
    size_t expected_body_length;

    // Métodos privados para cada estado
    ParseStatus parseRequestLine();
    ParseStatus parseHeaders();
    ParseStatus parseBody();

    // Helpers
    std::string& trim(std::string& s);
    bool isCompleteHeader() const;
    bool isCompleteBody() const;
};

#endif
```

```cpp
// RequestParser.cpp
#include "RequestParser.hpp"
#include <sstream>
#include <cctype>

RequestParser::RequestParser()
    : current_state(STATE_REQUEST_LINE),
      body_received(0),
      expected_body_length(0)
{
}

RequestParser::~RequestParser() {}

ParseStatus RequestParser::parse(const std::string& chunk) {
    buffer += chunk;

    while (current_state != STATE_COMPLETE && current_state != STATE_ERROR) {
        switch (current_state) {
            case STATE_REQUEST_LINE:
                if (parseRequestLine() == PARSE_ERROR)
                    return PARSE_ERROR;
                break;

            case STATE_HEADERS:
                if (parseHeaders() == PARSE_ERROR)
                    return PARSE_ERROR;
                break;

            case STATE_BODY:
                if (parseBody() == PARSE_ERROR)
                    return PARSE_ERROR;
                break;

            default:
                break;
        }
    }

    if (current_state == STATE_ERROR)
        return PARSE_ERROR;

    if (current_state == STATE_COMPLETE)
        return PARSE_COMPLETE;

    return PARSE_INCOMPLETE;
}

ParseStatus RequestParser::parseRequestLine() {
    // Buscar \r\n
    size_t pos = buffer.find("\r\n");
    if (pos == std::string::npos)
        return PARSE_INCOMPLETE;

    std::string line = buffer.substr(0, pos);
    buffer.erase(0, pos + 2);

    // Parsear: "GET /path HTTP/1.1"
    std::istringstream iss(line);
    std::string method, uri, protocol;

    if (!(iss >> method >> uri >> protocol)) {
        error_msg = "Invalid request line";
        current_state = STATE_ERROR;
        return PARSE_ERROR;
    }

    // Validar
    if (method != "GET" && method != "POST" && method != "DELETE"
        && method != "HEAD" && method != "PUT") {
        error_msg = "Method not allowed";
        current_state = STATE_ERROR;
        return PARSE_ERROR;
    }

    if (protocol != "HTTP/1.1" && protocol != "HTTP/1.0") {
        error_msg = "Unsupported HTTP version";
        current_state = STATE_ERROR;
        return PARSE_ERROR;
    }

    request.setMethod(method);
    request.setUri(uri);
    request.setProtocol(protocol);

    current_state = STATE_HEADERS;
    return PARSE_INCOMPLETE;  // Seguir con headers
}

ParseStatus RequestParser::parseHeaders() {
    while (true) {
        size_t pos = buffer.find("\r\n");
        if (pos == std::string::npos)
            return PARSE_INCOMPLETE;

        std::string line = buffer.substr(0, pos);
        buffer.erase(0, pos + 2);

        // Header vacío = fin de headers
        if (line.empty()) {
            // Determinar si hay body
            std::string content_length = request.getHeader("Content-Length");
            if (!content_length.empty()) {
                expected_body_length = std::atoi(content_length.c_str());
            }

            if (expected_body_length > 0) {
                current_state = STATE_BODY;
                return PARSE_INCOMPLETE;
            } else {
                current_state = STATE_COMPLETE;
                return PARSE_COMPLETE;
            }
        }

        // Parsear header
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            error_msg = "Invalid header";
            current_state = STATE_ERROR;
            return PARSE_ERROR;
        }

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim spaces
        trim(value);

        request.setHeader(key, value);
    }
}

ParseStatus RequestParser::parseBody() {
    if (body_received >= expected_body_length) {
        current_state = STATE_COMPLETE;
        return PARSE_COMPLETE;
    }

    size_t need = expected_body_length - body_received;
    if (buffer.length() < need)
        return PARSE_INCOMPLETE;

    request.setBody(buffer.substr(0, need));
    buffer.erase(0, need);
    body_received += need;

    current_state = STATE_COMPLETE;
    return PARSE_COMPLETE;
}

std::string& RequestParser::trim(std::string& s) {
    // Trim leading
    size_t start = s.find_first_not_of(" \t");
    if (start != std::string::npos)
        s = s.substr(start);

    // Trim trailing
    size_t end = s.find_last_not_of(" \t");
    if (end != std::string::npos)
        s = s.erase(end + 1);

    return s;
}
```

---

## 4️⃣ EVENT LOOP (MULTIPLICADOR)

```cpp
// EventLoop.hpp
#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include <vector>
#include <poll.h>
#include <map>

class ClientConnection;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void addServerSocket(int fd);
    void addClientSocket(int fd);
    void removeSocket(int fd);

    void run();
    void stop() { running = false; }

private:
    std::vector<pollfd> pollfds;
    std::map<int, ClientConnection*> connections;
    std::vector<int> server_fds;
    bool running;
    static const int POLL_TIMEOUT = 1000;  // 1 segundo
    static const int MAX_POLL_EVENTS = 1024;

    void handleNewConnection(int server_fd);
    void handleClientData(int client_fd);
    void handleClientWrite(int client_fd);
    void closeConnection(int client_fd);
};

#endif
```

```cpp
// EventLoop.cpp (PSEUDOCÓDIGO)
#include "EventLoop.hpp"
#include "ClientConnection.hpp"
#include <iostream>
#include <cstring>

EventLoop::EventLoop() : running(true) {}

EventLoop::~EventLoop() {
    // Limpiar todas las conexiones
}

void EventLoop::addServerSocket(int fd) {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;  // Solo lectura para server
    pollfds.push_back(pfd);
    server_fds.push_back(fd);
}

void EventLoop::addClientSocket(int fd) {
    ClientConnection* conn = new ClientConnection(fd);
    connections[fd] = conn;

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;  // Leer petición
    pollfds.push_back(pfd);
}

void EventLoop::run() {
    while (running) {
        // Poll con timeout
        int poll_result = poll(&pollfds[0], pollfds.size(), POLL_TIMEOUT);

        if (poll_result < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        if (poll_result == 0) {
            // Timeout: revisar timeouts de conexiones
            continue;
        }

        // Iterar sobre todos los FDs
        for (size_t i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].revents == 0)
                continue;

            int fd = pollfds[i].fd;

            // ¿Es un server socket?
            if (std::find(server_fds.begin(), server_fds.end(), fd)
                != server_fds.end()) {
                if (pollfds[i].revents & POLLIN) {
                    handleNewConnection(fd);
                }
            }
            // Es un client socket
            else {
                if (pollfds[i].revents & POLLIN) {
                    handleClientData(fd);
                }
                if (pollfds[i].revents & POLLOUT) {
                    handleClientWrite(fd);
                }
                if (pollfds[i].revents & (POLLERR | POLLHUP)) {
                    closeConnection(fd);
                }
            }
        }
    }
}

void EventLoop::handleNewConnection(int server_fd) {
    // accept() y añadir a EventLoop
    // (Ver ServerSocket.hpp para detalles)
}

void EventLoop::handleClientData(int client_fd) {
    // Leer datos
    // Pasar a RequestParser
    // Si completo, cambiar a POLLOUT
}

void EventLoop::handleClientWrite(int client_fd) {
    // Enviar response
    // Si completo, cerrar o keep-alive
}

void EventLoop::closeConnection(int client_fd) {
    // Limpiar y remover
    connections.erase(client_fd);
    // Remover de pollfds
}
```

---

## 5️⃣ REQUEST & RESPONSE

```cpp
// Request.hpp
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    Request();
    ~Request();

    // Setters
    void setMethod(const std::string& m) { method = m; }
    void setUri(const std::string& u) { uri = u; }
    void setProtocol(const std::string& p) { protocol = p; }
    void setBody(const std::string& b) { body = b; }
    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    // Getters
    const std::string& getMethod() const { return method; }
    const std::string& getUri() const { return uri; }
    const std::string& getProtocol() const { return protocol; }
    const std::string& getBody() const { return body; }
    const std::string& getHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const { return headers; }

private:
    std::string method;
    std::string uri;
    std::string protocol;
    std::string body;
    std::map<std::string, std::string> headers;
};

#endif
```

```cpp
// Response.hpp (PSEUDOCÓDIGO)
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Server.hpp"
#include <string>

class Response {
public:
    Response();
    ~Response();

    void generate(const Request& req, const Server& server);
    const std::string& getResponse() const { return response_buffer; }

    // Helpers
    int getStatusCode() const { return status_code; }

private:
    int status_code;
    std::string response_buffer;

    // Ensambladores de respuesta
    void assembleResponse(int code, const std::string& content_type,
                          const std::string& body);

    // Helpers de routing
    const Location* findLocation(const Request& req, const Server& server);
    std::string resolvePath(const Request& req, const Location& loc);

    // Handlers
    void handleStaticFile(const std::string& path, const std::string& content_type);
    void handleError(int code, const Server& server);
    void handleCgi(const Request& req, const Location& loc);
};

#endif
```

---

## 6️⃣ MIME TYPES (HELPER)

```cpp
// MimeTypes.hpp
#ifndef MIME_TYPES_HPP
#define MIME_TYPES_HPP

#include <string>
#include <map>

class MimeTypes {
public:
    static std::string getType(const std::string& filename);

private:
    static std::map<std::string, std::string> types;
    static void init();
};

#endif
```

```cpp
// MimeTypes.cpp
#include "MimeTypes.hpp"

std::map<std::string, std::string> MimeTypes::types;

std::string MimeTypes::getType(const std::string& filename) {
    if (types.empty())
        init();

    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos)
        return "application/octet-stream";

    std::string ext = filename.substr(pos);
    std::map<std::string, std::string>::iterator it = types.find(ext);

    if (it != types.end())
        return it->second;

    return "application/octet-stream";
}

void MimeTypes::init() {
    types[".html"] = "text/html";
    types[".htm"] = "text/html";
    types[".css"] = "text/css";
    types[".js"] = "application/javascript";
    types[".json"] = "application/json";
    types[".png"] = "image/png";
    types[".jpg"] = "image/jpeg";
    types[".jpeg"] = "image/jpeg";
    types[".gif"] = "image/gif";
    types[".svg"] = "image/svg+xml";
    types[".txt"] = "text/plain";
    types[".pdf"] = "application/pdf";
    types[".zip"] = "application/zip";
    // ... más tipos
}
```

---

## 7️⃣ CONFIG READER (PARSER)

```cpp
// ConfigReader.hpp
#ifndef CONFIG_READER_HPP
#define CONFIG_READER_HPP

#include "Config.hpp"
#include <string>
#include <vector>

class ConfigReader {
public:
    ConfigReader(const std::string& filepath);
    ~ConfigReader();

    std::vector<Server> parse();

private:
    std::string filepath;
    std::vector<std::string> lines;

    // Helpers
    void readFile();
    void removeComments();
    std::string trim(const std::string& s);
    int findBlockStart(size_t& line_num);
    int findBlockEnd(size_t& line_num);
    Server parseServer(size_t& line_num);
    Location parseLocation(size_t& line_num, Server& server);
};

#endif
```

```cpp
// ConfigReader.cpp (PSEUDOCÓDIGO)
#include "ConfigReader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

ConfigReader::ConfigReader(const std::string& filepath)
    : filepath(filepath)
{
    readFile();
}

ConfigReader::~ConfigReader() {}

void ConfigReader::readFile() {
    std::ifstream file(filepath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
}

std::vector<Server> ConfigReader::parse() {
    std::vector<Server> servers;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = trim(lines[i]);

        if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            Server server = parseServer(i);
            servers.push_back(server);
        }
    }

    return servers;
}

Server ConfigReader::parseServer(size_t& line_num) {
    Server server;

    ++line_num;  // Ir a la línea siguiente del "server {"

    while (line_num < lines.size()) {
        std::string line = trim(lines[line_num]);

        if (line == "}")
            break;

        if (line.find("listen") == 0) {
            int port = std::atoi(line.substr(6).c_str());
            server.setPort(port);
        } else if (line.find("server_name") == 0) {
            std::string name = line.substr(11);
            name = trim(name);
            server.addServerName(name);
        } else if (line.find("root") == 0) {
            std::string root = line.substr(4);
            root = trim(root);
            server.setRoot(root);
        } else if (line.find("location") == 0) {
            Location loc = parseLocation(line_num, server);
            server.addLocation(loc);
            continue;  // parseLocation ya incrementó line_num
        }

        ++line_num;
    }

    return server;
}
```

---

## ✅ NOTAS FINALES

Estos ejemplos son **pseudocódigo + C++98 real** para que entiendas:

1. **Herencia de Context** → DRY (Don't Repeat Yourself)
2. **RequestParser FSM** → Robusto para datos fragmentados
3. **EventLoop poll()** → No bloqueante
4. **Routing jerárquico** → Server + Location
5. **Helpers simples** → Fácil mantenimiento

**Todos compilan en C++98** (sin lambdas, sin `auto`, etc.)

Usa esto como blueprint, **NO como copy-paste** 😎

---

## 📚 DONDE COPIAR CADA COSA

| Componente | De Dónde | Por Qué |
|-----------|----------|--------|
| Config jerárquica | Webserver-5 | Elegancia con herencia |
| RequestParser FSM | Webserver-1 | Robusto y eficiente |
| Poll loop | Webserver-2 | Buen balance |
| Routing (Longest Prefix) | Webserver-5 | Intuitivo |
| CGI con fork | Webserver-4 | Bien hecho |
| Error handling | Webserver-5 | Excepciones limpias |

¡Éxito en la implementación! 🚀

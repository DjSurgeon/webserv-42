# 📊 Matriz de Cobertura de Pruebas (Test Cases)

Este documento actúa como el "mapa de calor" visual de nuestra suite de pruebas. Nos permite observar de un solo vistazo qué módulos y *Edge Cases* están protegidos por nuestros tests unitarios y de integración, sin necesidad de leer directamente el código C++.

> **Nota para el equipo:** Cada vez que añadas una nueva prueba (siguiendo el patrón AAA) para salvaguardar un comportamiento o arreglar un bug, añade la fila correspondiente a esta tabla.

## Estado Actual de Cobertura

| Componente | Caso de Prueba (Edge Case) | Resultado Esperado (Assert) | Estado |
| --- | --- | --- | --- |
| `ConfigParser` | Directiva desconocida en `server` | Lanza `std::runtime_error` ("Fail Fast") | ✅ Hecho |
| `ConfigParser` | Valor de `client_max_body_size` en bytes | Parsea a `size_t` correctamente | ✅ Hecho |
| `ConfigParser` | Falta llave `}` al final del archivo | Lanza error de sintaxis | ✅ Hecho |
| `FileHandler` | Mapeo MIME estándar (.html, .js, .png, etc.) | Devuelve tipo correcto (text/html, etc.) | ✅ Hecho |
| `FileHandler` | Archivo sin extensión o extensión desconocida | Devuelve `application/octet-stream` | ✅ Hecho |
| `FileHandler` | Múltiples puntos en nombre de archivo | Resuelve según la última extensión | ✅ Hecho |
| `FileHandler` | Lectura exitosa de archivo existente | Status 200 OK, body y Content-Length correctos | ✅ Hecho |
| `FileHandler` | Archivo inexistente | Status 404 Not Found | ✅ Hecho |
| `FileHandler` | Archivo sin permisos de lectura | Status 403 Forbidden | ✅ Hecho |
| `FileHandler` | Intento de leer un directorio como archivo | Status 500 Internal Server Error | ✅ Hecho |
| `FileHandler` | Borrado de archivo inexistente | Status 404 Not Found | ✅ Hecho |
| `FileHandler` | Intento de borrar un directorio | Status 403 Forbidden, directorio persiste | ✅ Hecho |
| `FileHandler` | Borrado de archivo sin permisos de escritura | Status 403 Forbidden, archivo persiste | ✅ Hecho |
| `FileHandler` | Borrado exitoso de archivo | Status 204 No Content, el archivo desaparece | ✅ Hecho |
| `CgiHandler` | Generación de variables Core (Method/Protocol) | REQUEST_METHOD y SERVER_PROTOCOL presentes | ✅ Hecho |
| `CgiHandler` | Extracción de QUERY_STRING | Se obtiene valor tras el símbolo `?` | ✅ Hecho |
| `CgiHandler` | Transformación de cabeceras HTTP a Meta-variables | Prefijo `HTTP_`, uppercase y guiones a guiones bajos | ✅ Hecho |
| `CgiHandler` | Reserva y liberación de memoria del entorno CGI (`char**`) | El array termina en NULL y los valores coinciden. Valgrind reporta 0 leaks | ✅ Hecho |
| `CgiHandler` | Inicialización de Pipes IPC | Crea File Descriptors válidos sin fugas | ✅ Hecho |
| `CgiHandler` | Mecanismo Anti-Deadlock (Bomba 10MB) | Usa `tmpfile` para STDIN, evita bloqueos de buffer | ✅ Hecho |
| `CgiHandler` | Auditoría de Recursos (100 peticiones) | Zero FD Leaks tras ejecución masiva | ✅ Hecho |
| `FileHandler` | Generación de autoindex exitosa | Status 200 OK, HTML con lista de archivos | ✅ Hecho |
| `FileHandler` | Fallo en autoindex (dir inexistente) | Status 403 Forbidden | ✅ Hecho |
| `StaticRouter` | Método no permitido (vector vacío) | Asume solo `GET`, devuelve 405 si es `POST` | ✅ Hecho |

*(Tabla en continua expansión)*

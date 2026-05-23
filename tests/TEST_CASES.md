# 📊 Matriz de Cobertura de Pruebas (Test Cases)

Este documento actúa como el "mapa de calor" visual de nuestra suite de pruebas. Nos permite observar de un solo vistazo qué módulos y *Edge Cases* están protegidos por nuestros tests unitarios y de integración, sin necesidad de leer directamente el código C++.

> **Nota para el equipo:** Cada vez que añadas una nueva prueba (siguiendo el patrón AAA) para salvaguardar un comportamiento o arreglar un bug, añade la fila correspondiente a esta tabla.

## Estado Actual de Cobertura

| Componente | Caso de Prueba (Edge Case) | Resultado Esperado (Assert) | Estado |
| --- | --- | --- | --- |
| `ConfigParser` | Directiva desconocida en `server` | Lanza `std::runtime_error` ("Fail Fast") | ✅ Hecho |
| `ConfigParser` | Valor de `client_max_body_size` en bytes | Parsea a `size_t` correctamente | ✅ Hecho |
| `ConfigParser` | Falta llave `}` al final del archivo | Lanza error de sintaxis | ✅ Hecho |
| `StaticRouter` | Método no permitido (vector vacío) | Asume solo `GET`, devuelve 405 si es `POST` | ✅ Hecho |

*(Tabla en continua expansión)*

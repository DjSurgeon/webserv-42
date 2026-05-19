# 🚀 Webserv-42 Development Log

Este documento registra de forma exhaustiva las decisiones arquitectónicas, los pasos de implementación y las pruebas realizadas durante el desarrollo del proyecto.

---

## 📅 Día 1: Fundamentos, Arquitectura y Sockets Seguros (RAII)

### 🎯 Objetivos del Día
1. Inicializar la estructura del proyecto y el sistema de compilación bajo las estrictas normas de la escuela 42 (`C++98`, sin librerías externas no permitidas).
2. Abstraer la creación y gestión de descriptores de archivo (FD) para los sockets utilizando el paradigma RAII (Resource Acquisition Is Initialization) para garantizar cero fugas (0 leaks).
3. Establecer un entorno de pruebas robusto para validar casos límite (*edge cases*).

### 🏗️ Fase 1: Inicialización del Entorno y Estructura
- **Estructura de Directorios:** Se creó una estructura modular para separar las responsabilidades del servidor:
  - `src/network/`: Manejo de bajo nivel (Sockets, EventLoop).
  - `src/http/`: Protocolo HTTP (Request, Parser, Response).
  - `src/config/`: Parseo y estructuración del `.conf`.
  - `src/handlers/`: Lógica de respuesta (Estáticos, CGI, Errores).
  - `src/utils/`: Utilidades genéricas.
  - `includes/`, `conf/`, `www/`, `tests/`.
- **Makefile:** Se diseñó un `Makefile` compatible con `C++98` con los flags obligatorios (`-Wall -Wextra -Werror -std=c++98`). Se implementaron las reglas estándar (`all`, `clean`, `fclean`, `re`).
- **Verificación Base:** Se creó un `src/main.cpp` mínimo para verificar que la cadena de compilación funcionara correctamente sin advertencias.

### 🔌 Fase 2: Encapsulación RAII del `ListeningSocket`
- **Problema:** El manejo crudo de descriptores de archivo en C++ es propenso a fugas si ocurren excepciones u olvidos de llamadas a `close()`.
- **Solución (RAII):** Se implementó la clase `ListeningSocket`.
  - **Constructor:** Llama a `socket(AF_INET, SOCK_STREAM, 0)`. Si falla (devuelve `-1`), lanza una `std::runtime_error`. Esto asegura que nunca exista una instancia de la clase en un estado inválido.
  - **Destructor:** Cierra automáticamente el descriptor con `close(_fd)`. Si falla, registra el error en `std::cerr` (los destructores no deben lanzar excepciones).
  - **Prevención de Copias:** Acatando el estándar `C++98`, se ocultaron (haciéndolos privados) el constructor de copia y el operador de asignación. Esto previene que dos objetos intenten cerrar el mismo FD, evitando errores críticos de doble liberación (*double-close*).

### 📡 Fase 3: Configuración de Bind, Listen y Port Reuse
- Se implementó el método público `init(int port)` en `ListeningSocket` para centralizar la configuración de red.
- **Port Reuse (`SO_REUSEADDR`):** Se utilizó `setsockopt` para permitir la reutilización de direcciones. Esto soluciona el problema de "Address already in use" del kernel, permitiendo reiniciar el servidor inmediatamente (crucial para el desarrollo rápido).
- **Binding:** Se configuró un `struct sockaddr_in` usando `AF_INET`, `INADDR_ANY` (escucha en todas las interfaces) y `htons(port)`.
- **Listening:** Se añadió la llamada `listen(_fd, 128)` para poner el socket en modo pasivo, permitiendo al sistema operativo encolar hasta 128 solicitudes de clientes simultáneas.
- **Manejo de Errores:** Tanto `setsockopt`, como `bind` y `listen` validan su retorno y lanzan excepciones descriptivas en caso de fallo.
- **Validación:** Se comprobó exitosamente el estado mediante la ejecución en segundo plano y el uso del comando `ss -lnt` / `lsof -i :8080`, confirmando el estado `LISTEN`.

### ⚡ Fase 4: Abstracción del `ClientSocket` (Non-Blocking)
- Se desarrolló la clase `ClientSocket` encargada de envolver el FD devuelto por `accept()`.
- **Modo No Bloqueante por Defecto:** Para cumplir con el requerimiento central del proyecto de multiplexación de E/S, el constructor aplica inmediatamente la bandera `O_NONBLOCK` mediante la llamada al sistema `fcntl()`. Si esto falla, lanza excepción.
- **Buffers:** Se inicializaron las variables privadas `_read_buffer` y `_write_buffer` (tipo `std::string`) para soportar lecturas parciales y escrituras fragmentadas, fundamentales en redes asíncronas.
- **RAII:** Al igual que `ListeningSocket`, se encarga de llamar a `close()` en el destructor de forma segura.

### 🧪 Fase 5: Batería de Pruebas y Edge Cases
Para garantizar la solidez de las clases de red antes de integrarlas en el Event Loop, se diseñó un entorno de pruebas aislado en `tests/`.

- **Script de Pruebas:** Se creó `tests/run_tests.sh` que compila un ejecutable independiente y lo somete a análisis con Valgrind (`--track-fds=yes` y `--leak-check=full`).
- **Casos Evaluados (`tests/test_sockets.cpp`):**
  1. **Privileged Port (ListeningSocket):** Intento de `bind` al puerto 80 sin ser root. Se capturó correctamente la excepción, impidiendo la caída del programa.
  2. **Double Bind (ListeningSocket):** Llamar a `init()` dos veces sobre la misma instancia. Se rechazó correctamente.
  3. **Invalid Port (ListeningSocket):** Se validó el comportamiento al pasar puertos fuera de rango (`999999`).
  4. **Invalid FD (ClientSocket):** Instanciación con FD `-1`. Bloqueado inmediatamente por el constructor.
  5. **Closed FD (ClientSocket):** Instanciación con FD `9999` (inexistente). `fcntl` devolvió fallo y se manejó con una excepción limpia.
  6. **File FD (ClientSocket):** Se pasó un archivo regular (en vez de un socket) para verificar el funcionamiento de `fcntl`.

- **Resultados de Calidad:**
  - Todas las excepciones esperadas fueron capturadas adecuadamente.
  - Valgrind confirmó **0 fugas de memoria** (0 leaks) en un flujo normal y bajo estrés de excepciones.
  - Valgrind confirmó **0 descriptores de archivo abiertos residualmente** al finalizar, demostrando que el patrón RAII es invulnerable.

---

## 📅 Día 2: Refactorización de Cabeceras, Inicialización Pasiva y Encapsulación de Buffers (Opción C)

### 🎯 Objetivos del Día
1. Minimizar las dependencias de cabeceras en los archivos `.hpp` trasladándolas a los `.cpp`.
2. Habilitar la inicialización pasiva de `ListeningSocket` para flexibilizar la creación de servidores múltiples sin perder seguridad RAII.
3. Asegurar la integridad de los buffers de red en `ClientSocket` mediante un modelo de protección asimétrica (Opción C) y constructor `explicit`.

### 🏗️ Fase 1: Desacoplamiento de Cabeceras
- Se eliminaron las inclusiones de `<sys/socket.h>`, `<unistd.h>` y `<stdexcept>` de `ListeningSocket.hpp`, ya que la firma de la clase solo maneja tipos primitivos.
- Se movieron estas inclusiones a `ListeningSocket.cpp` reduciendo la sobrecarga de compilación (*compilation overhead*) del módulo de red.

### 🔌 Fase 2: Robustecimiento de `ListeningSocket`
- **Inicialización Dual:**
  - **Constructor por Defecto:** Configura `_fd = -1`, permitiendo crear instancias en estado "pasivo" (por ejemplo, para arreglos o miembros de clase) antes de conocer el puerto definitivo.
  - **Constructor Explícito (`explicit`):** Toma un puerto e inicializa inmediatamente todo el socket. Marcado como `explicit` para evitar conversiones de tipo implícitas.
- **Robustez ante excepciones:** Se rediseñó `init(port)` para que, ante cualquier fallo en las llamadas del sistema (`setsockopt`, `bind`, `listen`), se invoque inmediatamente a `close(_fd)` y se restablezca `_fd = -1`, garantizando que el objeto nunca retenga FDs huérfanos ni fugas.
- **Prevención de Doble Enlace:** `init(port)` ahora arroja una excepción explícita a nivel de aplicación si se detecta que el socket ya ha sido inicializado.

### 📡 Fase 3: Encapsulación Asimétrica en `ClientSocket` (Opción C)
- **Constructor Explícito:** Se añadió la directiva `explicit` a `ClientSocket(int client_fd)` para evitar conversiones implícitas desde enteros planos (que habrían permitido, por ejemplo, pasar un entero por error cerrando descriptores críticos del sistema como stdin/stdout).
- **Liberación de FD ante fallo de fcntl:** Si la configuración no bloqueante (`fcntl`) falla en el constructor, se invoca a `close(_fd)` antes de lanzar la excepción para prevenir la fuga del FD aceptado.
- **Frontera de Datos Segura (Opción C):**
  - **Getters Constantes:** Los getters de buffers retornan ahora referencias constantes (`const std::string&`). Esto elimina la duplicación de memoria en el heap y permite la lectura directa por parte del Parser, pero evita la corrupción externa de los datos.
  - **Mutadores Controlados:** Se crearon métodos proxy exclusivos para modificar los buffers (`append_to_read_buffer`, `append_to_write_buffer`, `consume_read_buffer`, `clear_write_buffer`).
  - **Consumo Seguro:** `consume_read_buffer(bytes)` cuenta con lógica defensiva que previene desbordamientos de buffer o borrados inválidos si se le solicita consumir más bytes de los presentes.

### 🧪 Fase 4: Nuevas Pruebas en la Suite (`tests/test_sockets.cpp`)
- Se agregaron dos nuevas pruebas para validar la refactorización:
  1. **test_explicit_constructor:** Valida que la inicialización directa a través del constructor explícito funcione de forma correcta y bloquee re-inicializaciones posteriores.
  2. **test_client_socket_buffers:** Valida el flujo completo de inyección (`append`), lectura const, consumo ordenado y limpieza (`clear`) en los buffers de `ClientSocket`.
- Las pruebas volvieron a ser sometidas a **Valgrind**, reportando **0 leaks** y **0 FDs residuales abiertos**.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Habiendo cimentado la red al más bajo nivel de forma segura y encapsulada, el objetivo del próximo ciclo será la implementación del **Event Loop**.
- Integrar la llamada de multiplexación (`poll()`).
- Manejar conexiones entrantes masivas sin bloqueo.
- Enrutar los eventos `POLLIN` (para leer de clientes) e inyectar los bytes directamente a los buffers del `ClientSocket`.

---

## 📅 Día 3: Estructura de Datos Pasiva para Peticiones HTTP (HttpRequest)

### 🎯 Objetivos del Día
1. Implementar una estructura de datos pasiva (`HttpRequest`) robusta para almacenar de manera estructurada los elementos de una petición HTTP parseada (método, URI, versión, cabeceras y cuerpo).
2. Facilitar la consulta de los elementos mediante getters constantes y seguros que eviten copias innecesarias de memoria.
3. Habilitar la reutilización de objetos mediante un método de limpieza `clear()`.

### 🏗️ Fase 1: Creación de la clase `HttpRequest`
- **Declaración (`src/http/HttpRequest.hpp`):**
  - Se declararon atributos privados para el método, la URI, la versión de protocolo HTTP, el cuerpo y un mapa `std::map<std::string, std::string>` para las cabeceras.
  - Se definieron getters públicos constantes que devuelven referencias constantes a los atributos privados.
  - Se definieron mutadores (`set_method`, `set_uri`, etc.) e interfaces proxy para la adición de cabeceras (`add_header`), aislando la clase del parser mientras se permite su construcción.
  - Se declaró un método `clear()` para restablecer el estado interno.
- **Implementación (`src/http/HttpRequest.cpp`):**
  - Métodos simples de obtención y asignación.
  - `clear()` libera la memoria de los strings y del mapa de cabeceras usando `.clear()`.

### 🧪 Fase 2: Suite de Pruebas Unitarias (`tests/test_http_request.cpp`)
- Se creó una batería de pruebas independiente para verificar:
  1. Estado inicial vacío tras la instanciación.
  2. Almacenamiento y recuperación correcta de valores comunes (mismatch checks).
  3. Comportamiento correcto de la limpieza y restablecimiento del estado con `clear()`.
- La suite compila limpiamente bajo las banderas `-Wall -Wextra -Werror -std=c++98`.

---

## 📅 Día 3 (Parte 2): Esqueleto y Estados de la FSM de RequestParser

### 🎯 Objetivos de la Sesión
1. Definir la infraestructura base del procesador de peticiones `RequestParser`.
2. Establecer la enumeración `e_parser_state` con todos los estados necesarios para parsear peticiones HTTP de manera secuencial (Start, Method, URI, Version, Header Key, Header Value, Body, Complete, Error).
3. Diseñar una estructura base para `feed(char c)` usando un switch completo para evitar advertencias de compilación por falta de casos de enumeración.

### 🏗️ Fase 1: Creación de `RequestParser`
- **Declaración (`src/http/RequestParser.hpp`):**
  - Se definió el enum `e_parser_state` que servirá de motor para la FSM.
  - Se incluyeron los atributos privados `_state` y `_request` (instancia de `HttpRequest`).
  - Se definieron los métodos públicos `feed(char c)`, `get_state()`, `get_request()`, y `reset()`.
  - Se deshabilitó la copia en cumplimiento con las guías de diseño de C++98.
- **Implementación (`src/http/RequestParser.cpp`):**
  - El constructor inicializa el estado en `STATE_START`.
  - El switch-case de `feed` incluye todos los estados del enum para garantizar un flujo limpio y silencioso de warnings de compilación.
  - Se incluyó `(void)c` para silenciar el unused parameter.

### 🧪 Fase 2: Suite de Pruebas Unitarias (`tests/test_parser_skeleton.cpp`)
- Se diseñó un test rápido para asegurar que:
  1. El estado inicial empiece en `STATE_START`.
  2. Alimentar caracteres (`feed`) no provoque desbordamientos ni fallos de segmentación.
  3. `reset()` restaure el parser correctamente.
- La compilación es completamente limpia bajo `-Wall -Wextra -Werror -std=c++98`.

---

## 📅 Día 3 (Parte 3): Captura del Método HTTP en la FSM

### 🎯 Objetivos de la Sesión
1. Implementar las transiciones del estado inicial `STATE_START` para ignorar espacios y líneas en blanco al principio de una petición, previniendo errores por conexiones inactivas o paquetes fragmentados.
2. Programar el procesamiento dinámico carácter por carácter del método en `STATE_METHOD`.
3. Validar que la transición al siguiente estado (`STATE_URI`) se active tras encontrar un espacio en blanco y que cualquier carácter inválido aborte la máquina al estado `STATE_ERROR`.

### 🏗️ Fase 1: Programación de Transiciones en `RequestParser`
- **Cambios en `src/http/RequestParser.cpp`:**
  - Se introdujo un helper privado `is_alpha` para asegurar conformidad estricta US-ASCII de los caracteres alfabéticos.
  - En `STATE_START`, se programó el descarte de `' '`, `'\r'`, y `'\n'`. Cualquier carácter alfabético inicializa el método en la petición y cambia el estado a `STATE_METHOD`. Otros caracteres provocan la entrada a `STATE_ERROR`.
  - En `STATE_METHOD`, se acumulan los caracteres alfabéticos. El carácter de espacio `' '` redirige el parser a `STATE_URI`. Cualquier carácter que rompa el estándar HTTP (por ejemplo, números o signos) fuerza una transición defensiva inmediata a `STATE_ERROR`.

### 🧪 Fase 2: Batería de Pruebas (`tests/test_parser_method.cpp`)
- Se creó una suite de pruebas para evaluar:
  1. Parseo correcto del método GET clásico con espacio final (`"GET "`).
  2. Omisión exitosa de espacios o saltos de línea al principio (`"  \r\n  POST "`).
  3. Rechazo de caracteres no permitidos en el método (por ejemplo, dígitos `"GE1 "`).
  4. Bloqueo de entrada en `STATE_ERROR` una vez que ocurre un fallo.
- Verificada la compilación en C++98 libre de warnings y leaks.

---

## 📅 Día 3 (Parte 4): Captura de la URI HTTP en la FSM

### 🎯 Objetivos de la Sesión
1. Implementar el procesamiento carácter por carácter de la URI en `STATE_URI`.
2. Validar que la transición al siguiente estado (`STATE_VERSION`) ocurra únicamente al encontrar un espacio en blanco delimitador y que la URI no esté vacía.
3. Prevenir violaciones del protocolo detectando caracteres de control (ASCII < 32) o caracteres eliminados (ASCII 127), abortando la FSM a `STATE_ERROR`.

### 🏗️ Fase 1: Programación de Transiciones en `RequestParser`
- **Cambios en `src/http/RequestParser.cpp`:**
  - Se introdujo el helper estático `is_uri_char` para validar caracteres ASCII visibles en el rango de `33` a `126` (inclusive).
  - En `STATE_URI`, la presencia de un espacio `' '` comprueba si el campo de URI está vacío. Si lo está (como en el caso de dos espacios seguidos `"GET  "`), cambia a `STATE_ERROR`; de lo contrario, cambia a `STATE_VERSION`.
  - Los caracteres válidos se concatenan al string de URI en `_request`. Cualquier carácter fuera del rango ASCII visible (caracteres de control, `DEL`, o no-ASCII) provoca una transición a `STATE_ERROR`.

### 🧪 Fase 2: Suite de Pruebas (`tests/test_parser_uri.cpp`)
- Se creó una suite de pruebas para evaluar:
  1. Captura correcta de la URI (`"/index.html"`) y salto a `STATE_VERSION`.
  2. Detección de errores ante URIs vacías (`"GET  "`).
  3. Rechazo de caracteres de control como `BEL` (ASCII 7).
  4. Rechazo del carácter `DEL` (ASCII 127).
- Verificada la compilación exitosa en C++98 libre de fallos y advertencias.

---

## 📅 Día 3 (Parte 5): Validación de Versión HTTP y Fin de Request-Line

### 🎯 Objetivos de la Sesión
1. Implementar la validación carácter por carácter de la versión HTTP en `STATE_VERSION` asegurando que coincida estrictamente con `"HTTP/1.1"`.
2. Manejar la secuencia de escape de fin de línea (`\r\n`) de manera segura y no bloqueante.
3. Transicionar al estado `STATE_HEADER_KEY` una vez cerrada la línea de petición, y derivar a `STATE_ERROR` ante cualquier carácter malformado o salto de línea prematuro.

### 🏗️ Fase 1: Modificaciones en `RequestParser`
- **Cambios en `src/http/RequestParser.hpp`:**
  - Se agregó la bandera booleana `_expect_newline` al bloque de variables privadas de la clase.
- **Cambios en `src/http/RequestParser.cpp`:**
  - Inicialización y reseteo de `_expect_newline` a `false`.
  - En `STATE_VERSION`, si `_expect_newline` es `false`: se valida la entrada contra la constante `"HTTP/1.1"` basándose en el índice actual de longitud del string acumulado. Si se recibe `\r` y el string tiene longitud 8, se activa `_expect_newline = true`.
  - Si `_expect_newline` es `true`: se exige recibir `\n` para desactivar la bandera y cambiar al estado `STATE_HEADER_KEY`. Cualquier otro carácter rompe el estado moviendo la FSM a `STATE_ERROR`.

### 🧪 Fase 2: Batería de Pruebas (`tests/test_parser_version.cpp`)
- Se creó una suite de pruebas que valida:
  1. Extracción exitosa de toda la línea de petición (`"GET /index.html HTTP/1.1\r\n"`) y llegada correcta a `STATE_HEADER_KEY`.
  2. Rechazo de versiones HTTP antiguas (`"HTTP/1.0"`).
  3. Rechazo de sintaxis incorrectas (`"HTTP/1.1a"` o overflows).
  4. Rechazo ante la ausencia del retorno de carro (`\r`).
  5. Rechazo ante caracteres inválidos tras el retorno de carro (espacio en lugar de `\n`).
- Validado bajo flags C++98 estrictos.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Retomar la FSM en `RequestParser` para capturar secuencialmente las cabeceras de tipo `Key: Value\r\n` y almacenarlas en el mapa de cabeceras de la petición (que ahora las normaliza automáticamente a minúsculas) hasta llegar al final del bloque de cabeceras (`\r\n`).

---

## 📅 Día 3 (Parte 6): Normalización de Cabeceras HTTP (Case-Insensitivity)

### 🎯 Objetivos de la Sesión
1. Implementar la normalización a minúsculas de las claves de las cabeceras HTTP de forma automatizada al guardarlas en `HttpRequest` para cumplir el estándar de insensibilidad a mayúsculas/minúsculas de HTTP/1.1.
2. Evitar la saturación de líneas en `add_header` abstrayendo la lógica en una función auxiliar limpia.

### 🏗️ Fase 1: Modificaciones en `HttpRequest`
- **Cambios en `src/http/HttpRequest.cpp`:**
  - Se incluyó la cabecera `<cctype>`.
  - Se programó la función auxiliar estática de archivo `to_lower` que procesa un string y retorna su equivalente completamente en minúsculas.
  - Se actualizó `HttpRequest::add_header` para aplicar `to_lower(key)` de manera transparente antes de insertar en el mapa `_headers`.

### 🧪 Fase 2: Actualización de Pruebas Unitarias (`tests/test_http_request.cpp`)
- Se actualizaron las aserciones de cabeceras en `test_setters_and_getters` para realizar las búsquedas (`find`) utilizando claves en minúscula (`"host"`, `"content-type"`, `"content-length"`), verificando que la normalización funciona de extremo a extremo.
- Verificado el correcto funcionamiento del conjunto completo de pruebas con `./tests/run_all_tests.sh`.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Una vez optimizado el parser con el búfer acumulador, pasaremos a programar los estados de cabeceras `STATE_HEADER_KEY` y `STATE_HEADER_VALUE` utilizando `_storage_buffer` para delimitar y rellenar las cabeceras en la petición, detectando el cierre del bloque de cabeceras al encontrar la línea vacía (`\r\n\r\n`).

---

## 📅 Día 3 (Parte 7): Integración del Búfer Acumulador en RequestParser

### 🎯 Objetivos de la Sesión
1. Optimizar el rendimiento y el uso de memoria de `RequestParser` mediante un string acumulador intermedio (`_storage_buffer`) para evitar reasignaciones en cada byte leído.
2. Preparar el parser para la extracción de cabeceras asimétricas (donde el almacenamiento en `HttpRequest` requiere poseer la clave y el valor completos de forma simultánea).

### 🏗️ Fase 1: Refactorización Estructural en `RequestParser`
- **Cambios en `src/http/RequestParser.hpp`:**
  - Se agregaron las variables privadas `_storage_buffer` y `_current_header_key`.
- **Cambios en `src/http/RequestParser.cpp`:**
  - `feed(char c)`: Se rediseñó la lógica de acumulación de los estados `STATE_START`, `STATE_METHOD`, `STATE_URI` y `STATE_VERSION` para empujar caracteres a `_storage_buffer` de manera directa.
  - En los caracteres delimitadores (espacio o retorno de carro `\r`), se valida el contenido acumulado, se asigna en `_request` mediante los setters correspondientes de una sola vez, y se limpia `_storage_buffer`.
  - `reset()`: Se incluyó el reinicio de las variables miembro `_storage_buffer` y `_current_header_key`.

### 🧪 Fase 2: Verificación y Pruebas Unitarias
- Se corrió el script automatizado `./tests/run_all_tests.sh` para verificar el correcto funcionamiento de todas las suites unitarias tras la refactorización.
- La compilación continúa libre de warnings y fallos de lógica bajo C++98 estándar.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Una vez validada la integración temprana de la Request-Line y los búferes de red, comenzaremos con la captura del bloque de cabeceras HTTP de tipo clave-valor (`STATE_HEADER_KEY` y `STATE_HEADER_VALUE`) utilizando el búfer acumulador hasta detectar la línea vacía de cierre del bloque (`\r\n\r\n`).

---

## 📅 Día 3 (Parte 8): Prueba de Humo de Integración Temprana (ClientSocket + RequestParser)

### 🎯 Objetivos de la Sesión
1. Validar la interoperabilidad y flujo de datos entre el búfer de red asíncrono (`ClientSocket`) y la FSM del parser (`RequestParser`).
2. Demostrar la consistencia del borrado de bytes procesados mediante `consume_read_buffer` evitando desalineaciones de memoria o fugas.
3. Asegurar que las ráfagas fragmentadas y ataques de entrada (como versiones incorrectas u espacios dobles) se manejen defensivamente en el pipeline conjunto.

### 🏗️ Fase 1: Creación de Suite de Integración
- **Cambios en `tests/test_integration.cpp` [NEW]:**
  - Implementación del pipeline `run_integration_pipeline` simulando la ráfaga de red del EventLoop real.
  - Creación de 5 tests específicos: petición estándar, fragmentación en red de 3 ráfagas, tolerancia a líneas vacías iniciales (RFC 7230), rechazo de múltiples espacios de separación y rechazo de versiones HTTP no admitidas.
- **Cambios en `tests/run_all_tests.sh` [MODIFY]:**
  - Incorporación automática de `test_integration` compilandola con `ClientSocket.cpp`, `HttpRequest.cpp` y `RequestParser.cpp`.

### 🧪 Fase 2: Ejecución de Pruebas
- Se corrió el suite completo mediante `./tests/run_all_tests.sh`. Todos los tests (incluido el de integración) pasaron con éxito y se validaron bajo las restricciones rigurosas de C++98.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Una vez resuelto el problema de descriptores cerrados y con todos los tests unitarios e integrados pasando al 100%, comenzaremos con la captura del bloque de cabeceras HTTP de tipo clave-valor (`STATE_HEADER_KEY` y `STATE_HEADER_VALUE`) utilizando el búfer acumulador hasta detectar la línea vacía de cierre del bloque (`\r\n\r\n`).

---

## 📅 Día 3 (Parte 8): Prueba de Humo de Integración Temprana (ClientSocket + RequestParser)

### 🎯 Objetivos de la Sesión
1. Validar la interoperabilidad y flujo de datos entre el búfer de red asíncrono (`ClientSocket`) y la FSM del parser (`RequestParser`).
2. Demostrar la consistencia del borrado de bytes procesados mediante `consume_read_buffer` evitando desalineaciones de memoria o fugas.
3. Asegurar que las ráfagas fragmentadas y ataques de entrada se manejen defensivamente en el pipeline conjunto.
4. Resolver el conflicto de descriptores cerrados debido a que la destrucción del primer `ClientSocket` cierra el descriptor de entrada (`stdin` / FD 0), provocando fallos en `fcntl` en los siguientes tests.

### 🏗️ Fase 1: Creación y Fix de la Suite de Integración
- **Cambios en `tests/test_integration.cpp` [NEW]:**
  - Implementación del pipeline `run_integration_pipeline` simulando la ráfaga de red del EventLoop real.
  - Creación de 5 tests específicos: petición estándar, fragmentación en red de 3 ráfagas, tolerancia a líneas vacías iniciales (RFC 7230), rechazo de múltiples espacios de separación y rechazo de versiones HTTP no admitidas.
  - **Corrección de conflicto de FD:** Se introdujo la función auxiliar `get_mock_fd()` que abre un descriptor único apuntando a `/dev/null` para cada prueba. Así, la destrucción del objeto en cada test cierra un descriptor independiente, eliminando los fallos de `fcntl(..., O_NONBLOCK)`.
- **Cambios en `tests/run_all_tests.sh` [MODIFY]:**
  - Incorporación automática de `test_integration` compilandola con `ClientSocket.cpp`, `HttpRequest.cpp` y `RequestParser.cpp`.

### 🧪 Fase 2: Ejecución de Pruebas
- Se corrió el suite completo mediante `./tests/run_all_tests.sh`. Todos los tests (incluido el de integración) pasaron con éxito y se validaron bajo las restricciones rigurosas de C++98.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Una vez refactorizada la FSM para delegar la lógica en funciones miembro privadas, la base del parser es sumamente modular. La siguiente tarea consistirá en implementar los manejadores de estado para las cabeceras HTTP (`STATE_HEADER_KEY` y `STATE_HEADER_VALUE`) utilizando esta nueva arquitectura limpia.

---

## 📅 Día 3 (Parte 9): Refactorización Modular de RequestParser a Funciones Miembro Privadas

### 🎯 Objetivos de la Sesión
1. Prevenir el crecimiento desmedido y la complejidad ciclomática del método `feed(char c)` delegando la lógica de transición a funciones miembro privadas dedicadas.
2. Mantener la complejidad temporal idéntica ($O(1)$) y la seguridad del análisis carácter a carácter para mitigar desbordamientos.

### 🏗️ Fase 1: Modularización Estructural
- **Cambios en `src/http/RequestParser.hpp` [MODIFY]:**
  - Se declararon las funciones miembro de ayuda `_handle_state_start`, `_handle_state_method`, `_handle_state_uri`, `_handle_state_version` y `_handle_global_newline`.
- **Cambios en `src/http/RequestParser.cpp` [MODIFY]:**
  - Se redefinió `feed(char c)` para interceptar de forma centralizada los saltos de línea esperados (`_expect_newline`) y despachar las transiciones de estado a través de la tabla `switch-case` en una sola línea por caso.
  - Se implementó cada manejador de estado preservando las validaciones estrictas y la acumulación en `_storage_buffer`.
  - Se conservaron las funciones auxiliares de archivo estáticas `is_alpha` y `is_uri_char` para mejorar la legibilidad y evitar repeticiones.
  - **Traducción y Documentación Doxygen:** Se eliminaron todos los comentarios inline dentro de las funciones para limpiar el código, traduciendo toda la documentación al inglés e implementando bloques formales de Doxygen (`/** ... */`) encima de cada función.

### 🧪 Fase 2: Ejecución de la Suite de Pruebas
- Se ejecutó `./tests/run_all_tests.sh`. Todos los tests compilaron sin warnings y pasaron de forma limpia en WSL, confirmando que la modularización no introdujo ninguna regresión.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Proceder con la implementación de `STATE_HEADER_VALUE` delegando a su respectivo manejador privado `_handle_state_header_value` en `RequestParser`.

---

## 📅 Día 3 (Parte 10): Implementación del Parseo de Claves de Cabecera y Detección de Doble CRLF

### 🎯 Objetivos de la Sesión
1. Aislar las claves de cabeceras HTTP carácter a carácter de forma asíncrona en `STATE_HEADER_KEY`.
2. Validar sintácticamente los caracteres permitidos (alfanuméricos y guiones) de forma estricta.
3. Detectar secuencias de doble CRLF (`\r\n\r\n`) para transicionar al cuerpo del mensaje (`STATE_BODY`).

### 🏗️ Fase 1: Desarrollo del Parser de Cabeceras
- **Cambios en `src/http/RequestParser.hpp` [MODIFY]:**
  - Declaración del manejador privado `_handle_state_header_key(char c)`.
- **Cambios en `src/http/RequestParser.cpp` [MODIFY]:**
  - Implementación de la función auxiliar estática `is_header_key_char(char c)` para validar caracteres válidos según la RFC.
  - Implementación de `_handle_state_header_key(char c)` acumulando en `_storage_buffer`, aislando la clave en `_current_header_key` al leer `:`, y detectando `\r` al inicio de línea para configurar `_expect_newline`.
  - Actualización de `_handle_global_newline(char c)` para saltar de `STATE_HEADER_KEY` a `STATE_BODY` tras detectar el doble CRLF.
  - Actualización de la tabla switch en `feed(char c)`.

### 🧪 Fase 2: Automatización y Pruebas Unitarias
- **Cambios en `tests/test_parser_header_key.cpp` [NEW]:**
  - Creación de pruebas unitarias que cubren: flujo feliz de lectura de clave, detección correcta de doble CRLF, error por clave vacía, y error por caracteres inválidos (como espacios intermedios).
- **Cambios en `tests/run_all_tests.sh` [MODIFY]:**
  - Incorporación de `test_parser_header_key` a la lista de compilación y ejecución automática.
- Se corrió `./tests/run_all_tests.sh` en WSL y todas las pruebas pasaron satisfactoriamente.

---

### ⏭️ Siguientes Pasos (Próxima Fase)
Implementar el parseo de valores de cabeceras (`STATE_HEADER_VALUE`) acumulando la cadena, gestionando los espacios en blanco iniciales opcionales (OWS) y registrando el par cabecera en el objeto `HttpRequest` al llegar el final de línea CRLF.

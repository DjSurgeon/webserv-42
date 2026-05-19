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

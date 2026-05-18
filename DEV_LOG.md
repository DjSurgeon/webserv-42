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

### ⏭️ Siguientes Pasos (Próxima Fase)
Habiendo cimentado la red al más bajo nivel de forma segura, el objetivo del próximo ciclo será la implementación del **Event Loop**.
- Integrar la llamada de multiplexación (`poll()`).
- Manejar conexiones entrantes masivas sin bloqueo.
- Enrutar los eventos `POLLIN` (para leer de clientes) y delegarlos de vuelta a los buffers del `ClientSocket`.

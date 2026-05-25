# 🧪 Webserv-42 Testing Guide

¡Bienvenido a la suite de pruebas de Webserv-42! Como ciudadanos de primera clase en este repositorio, los tests son la red de seguridad que garantiza que nuestro servidor HTTP es robusto, está libre de fugas de memoria y cumple con los estándares RFC, incluso bajo ataques masivos.

Cualquier miembro del equipo debe poder leer, ejecutar y añadir pruebas en menos de 2 minutos siguiendo esta guía.

---

## 🚀 1. Ejecución de Pruebas (El "Cómo")

### Dependencias
Nuestro proyecto está estrictamente escrito en **C++98 puro**. No requerimos la instalación de frameworks pesados de pruebas (como GoogleTest o Catch2). Todo el arnés de pruebas es *custom* e integrado en nuestros scripts bash.

### Comandos de Ejecución Principales

El proyecto cuenta con una batería de pruebas dividida en tres niveles de profundidad. Para una validación completa antes de un *commit*, debes ejecutar los tres:

1. **Suite de Pruebas Unitarias y Estructurales (make test)**
   Compila y ejecuta todos los tests atómicos aislados. Esto incluye el parseo de configuración, validación de rutas estáticas, generación de respuestas HTTP, manejo de archivos y resolución de procesos CGI.
   ```bash
   make test
   # O alternativamente:
   ./tests/scripts/run_all_tests.sh
   ```

2. **Test de Concurrencia a Nivel de Sistema**
   Lanza el binario `webserv` en segundo plano y dispara 20 clientes concurrentes asíncronos para asegurar que el `EventLoop` multiplexa correctamente sin bloqueos.
   ```bash
   ./tests/scripts/test_concurrency.sh
   ```

3. **Batería de Estrés Masivo y Ataques de Red**
   Utiliza un cliente C++ personalizado (`stress_client`) para lanzar ataques dirigidos contra el servidor:
   - **Flood:** Cientos de peticiones válidas simultáneas.
   - **Garbage:** Inyección de binarios y directivas HTTP inválidas.
   - **Drop:** Desconexión abrupta de sockets a mitad de transmisión.
   - **Slowloris:** Envío de 1 byte por segundo para intentar agotar los File Descriptors.
   ```bash
   ./tests/scripts/run_stress_tests.sh
   ```

> 💡 **Tip:** Para ejecutar la validación total del sistema con un solo comando:
> `make all && ./tests/scripts/run_all_tests.sh && ./tests/scripts/run_stress_tests.sh`

---

## 🏗️ 2. Arquitectura de las Pruebas Actuales

El proyecto actualmente cuenta con **19 suites de pruebas** (estado: 100% PASS), agrupadas por dominios lógicos:

### A. Configuración (`tests/config/`)
- `test_context`: Valida la herencia y los valores por defecto.
- `test_location_config` / `test_server_config`: Verifica la clonación profunda (Orthodox Canonical Form), el enrutamiento por *Longest Prefix Match* y la gestión de punteros.
- `test_config_parser`: Pruebas de preprocesamiento (eliminación de espacios/comentarios), validación estructural de bloques anidados y detección estricta de errores de sintaxis en archivos `.conf`.

### B. Manejadores (`tests/handlers/`)
- `test_static_router`: Valida la traducción de URIs a rutas físicas (ej. root + path), normalización de barras (`/`) y validación de métodos HTTP permitidos (405).
- `test_file_handler`: Pruebas de resolución de tipos MIME, permisos de lectura (403), NotFound (404), borrado físico de archivos y auto-generación de índices de directorios (`autoindex`).
- `test_cgi_handler`: Valida la correcta generación del entorno (variables `HTTP_*`), bifurcación de procesos (`fork/execve`), redirección IPC (Pipes/Tempfiles) y parseo defensivo de la salida del CGI (detectando `Status:` y previniendo caídas con salidas malformadas).

### C. Protocolo HTTP (`tests/http/`)
- `test_http_request` / `test_http_response`: Validación de setters/getters y correcta serialización de respuestas al estándar HTTP/1.1 (CRLF).
- `test_parser_*`: Batería exhaustiva de la Máquina de Estados Finita (FSM) que parsea métodos, URIs, versiones, cabeceras y cuerpos de forma no bloqueante, fragmentada y tolerante a fallos.

### D. Red e Integración (`tests/network/` & `tests/integration/`)
- `test_sockets`: Comprueba la gestión RAII de los sockets no bloqueantes (FDs).
- `test_integration`: Une los sockets de cliente con el parser HTTP para simular latencia de red.
- `test_parser_stress`: Valida la protección contra vulnerabilidades comunes (Buffer Overflows, Double Spaces, Negative Content-Lengths).

---

## 🛠️ 3. El Patrón "AAA" (El "Por qué")

La regla de oro del Tech Lead es clara: *"Un test que falla y no se entiende por qué ha fallado, es peor que no tener test."*

Para evitar el código espagueti en nuestras pruebas, **adoptamos una convención estricta: el patrón AAA (Arrange, Act, Assert)**. Cada test unitario que se añada a este proyecto debe estar visualmente separado en estas tres fases exactas usando comentarios.

### Ejemplo de Implementación Estándar

```cpp
void test_missing_semicolon_throws_error() {
    // 1. ARRANGE (Preparar: Given)
    // Qué necesitamos para que este test funcione.
    std::string bad_conf = "server { root /var/www }"; 
    create_temp_file("bad.conf", bad_conf);

    // 2. ACT (Actuar: When)
    // La acción específica que estamos poniendo a prueba.
    bool exception_thrown = false;
    try {
        ConfigParser parser("bad.conf");
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
    }

    // 3. ASSERT (Comprobar: Then)
    // El resultado innegociable. Si esto falla, el test falla.
    assert(exception_thrown == true);
    
    // (Limpieza)
    delete_temp_file("bad.conf");
}
```
*Si vas a crear un test nuevo, copia y pega esta estructura.*

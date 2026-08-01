# 🧪 Webserv-42 Testing Guide

¡Bienvenido a la suite de pruebas de Webserv-42! Como ciudadanos de primera clase en este repositorio, los tests son la red de seguridad que garantiza que nuestro servidor HTTP es robusto, está libre de fugas de memoria y cumple con los estándares RFC, incluso bajo ataques masivos.

Cualquier miembro del equipo debe poder leer, ejecutar y añadir pruebas en menos de 2 minutos siguiendo esta guía.

---

## 🚀 1. Ejecución de Pruebas (El "Cómo")

### Dependencias
Nuestro proyecto está estrictamente escrito en **C++98 puro**. No requerimos la instalación de frameworks pesados de pruebas (como GoogleTest o Catch2). Todo el arnés de pruebas es *custom* e integrado en nuestros scripts bash.

### Comandos de Ejecución Principales

El proyecto cuenta con una batería de pruebas dividida en tres niveles de profundidad. Para una validación completa antes de un *commit*, debes ejecutar los tres:

1. **Suite de Pruebas Unitarias e Integradas (make test)**
   Compila y ejecuta todos los tests atómicos aislados. Esto incluye el parseo de configuración, validación de rutas estáticas, generación de respuestas HTTP, manejo de archivos, cookies y procesos CGI.
   ```bash
   make test
   # O alternativamente:
   ./tests/scripts/run_all_tests.sh
   ```

2. **Test de Concurrencia a Nivel de Sistema**
   Lanza el binario `webserv` en segundo plano y dispara 20 clientes concurrentes asíncronos para asegurar que el `EventLoop` multiplexa correctamente sin bloqueos. Se utiliza `Connection: close` para garantizar la terminación limpia de cada socket.
   ```bash
   ./tests/scripts/test_concurrency.sh
   ```

3. **Batería de Estrés Masivo y Ataques de Red**
   Utiliza un cliente C++ personalizado (`stress_client`) para lanzar ataques dirigidos contra el servidor:
   - **Flood:** Cientos de peticiones válidas simultáneas.
   - **Garbage:** Inyección de binarios y directivas HTTP inválidas.
   - **Drop:** Desconexión abrupta de sockets a mitad de transmisión.
   - **Slowloris:** Envío de 1 byte por segundo para intentar agotar los File Descriptors.
   - **Delete:** Operaciones de borrado físico masivo en paralelo.
   ```bash
   ./tests/scripts/run_stress_tests.sh
   ```

> 💡 **Tip:** Para ejecutar la validación total del sistema con un solo comando:
> `make all && ./tests/scripts/run_all_tests.sh && ./tests/scripts/run_stress_tests.sh`

---

## 🏗️ 2. Arquitectura de las Pruebas Actuales

El proyecto actualmente cuenta con **20 suites de pruebas** (estado: 100% PASS), agrupadas por dominios lógicos:

### A. Configuración (`tests/config/`)
- `test_context`: Valida la herencia y los valores por defecto.
- `test_location_config` / `test_server_config`: Verifica la clonación profunda (OCF), el enrutamiento por *Longest Prefix Match* y la gestión de punteros.
- `test_config_parser`: Pruebas de preprocesamiento, validación estructural de bloques y detección estricta de errores (ej. puertos inválidos como 999999).

### B. Manejadores (`tests/handlers/`)
- `test_static_router`: Valida la traducción de URIs a rutas físicas y validación de métodos (405).
- `test_file_handler`: Pruebas de tipos MIME, NotFound (404), Forbidden (403), borrado físico y `autoindex`.
- `test_cgi_handler`: Valida la generación de entorno `HTTP_*`, fork/execve, Pipes/Tempfiles y parseo defensivo de la salida del CGI (Status pseudo-header).

### C. Protocolo HTTP (`tests/http/`)
- `test_http_request` / `test_http_response`: Validación de setters/getters y serialización estándar.
- `test_http_cookies`: **(Nuevo)** Verifica el parseo de la cabecera `Cookie` (múltiples pares, espacios extra, etc.).
- `test_http_response_cookies`: Integrado en `test_http_response`, verifica que múltiples `Set-Cookie` coexisten correctamente en la respuesta final.
- `test_parser_*`: Batería exhaustiva de la Máquina de Estados Finita (FSM) no bloqueante.

### D. Red e Integración (`tests/network/` & `tests/integration/`)
- `test_sockets`: Comprueba la gestión RAII de los sockets no bloqueantes.
- `test_integration`: Une sockets de cliente con el parser HTTP simulando latencia.
- `test_parser_stress`: Protección contra Buffer Overflows, Double Spaces y Negative Content-Lengths.

---

## 🛡️ 3. Puertas de Calidad (Quality Gates)

No solo nos importa que el código funcione, sino que esté **bien escrito** y sea **seguro**.

1. **Estilo Google/42 (cpplint & clang-format)**
   Todo el código debe pasar el linter sin errores. No se permiten referencias no-constantes (usar punteros para parámetros de salida) ni violaciones de formato.
   ```bash
   find src tests -name "*.*pp" | xargs cpplint
   clang-format --dry-run --Werror $(find src tests -name "*.*pp")
   ```

2. **Memoria y FD Leak (Valgrind)**
   El servidor es verificado en el CI/CD bajo Valgrind para asegurar 0 fugas de memoria y 0 descriptores de archivo abiertos tras el cierre.

3. **CI/CD (GitHub Actions)**
   Cada Pull Request dispara automáticamente la compilación estricta, los linters y las pruebas de Valgrind.

---

## 🛠️ 4. El Patrón "AAA" (El "Por qué")

Adoptamos el patrón **AAA (Arrange, Act, Assert)** para mantener los tests legibles y mantenibles.

```cpp
void test_cookie_parsing() {
    // 1. ARRANGE
    HttpRequest req;
    req.add_header("Cookie", "user=serjimen; theme=dark");

    // 2. ACT
    const std::map<std::string, std::string>& cookies = req.get_cookies();

    // 3. ASSERT
    assert(cookies.at("user") == "serjimen");
    assert(cookies.at("theme") == "dark");
}
```

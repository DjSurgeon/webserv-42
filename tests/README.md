# 🧪 Webserv-42 Testing Guide

¡Bienvenido a la suite de pruebas de Webserv-42! Como ciudadanos de primera clase en este repositorio, los tests son la red de seguridad que garantiza que nuestro servidor HTTP es robusto y está libre de regresiones.

Cualquier miembro del equipo debe poder leer, ejecutar y añadir pruebas en menos de 2 minutos siguiendo esta guía.

---

## 🚀 1. Ejecución de Pruebas (El "Cómo")

### Dependencias
Nuestro proyecto está estrictamente escrito en **C++98 puro**. No requerimos la instalación de frameworks pesados de pruebas (como GoogleTest o Catch2). Todo el arnés de pruebas es *custom* e integrado en nuestros scripts bash.

### Comandos de Ejecución
Existen múltiples formas de lanzar las pruebas dependiendo de tu enfoque:

- **Suite Completa Automatizada**: Compila y ejecuta absolutamente todos los archivos de prueba unitarios y de integración.
  ```bash
  make test
  # O alternativamente:
  ./tests/run_all_tests.sh
  ```

- **Pruebas Específicas**: Si solo estás depurando el parser o un componente concreto.
  ```bash
  ./tests/run_tests.sh
  ```

- **Pruebas de Estrés y Fragmentación**: Evalúa la FSM bajo asedio de red y condiciones extremas.
  ```bash
  ./tests/run_stress_tests.sh
  ```

---

## 🛠️ 2. El Patrón "AAA" (El "Por qué")

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

---

## 🌐 3. Testing Manual y Sandbox (Verificación Local)

Antes de hacer un Pull Request, es obligatorio verificar los módulos atómicos con estos flujos de trabajo manuales si se están tocando capas de bajo nivel.

### 🔌 Testing de la Capa de Red (Network Layer)
Para verificar que `ListeningSocket` y `ClientSocket` aceptan conexiones sin bloquear el hilo principal:

1. Compila y ejecuta el binario principal: `./webserv`
2. Abre un terminal separado y dispara una conexión cruda usando `netcat` (`nc`):
   ```bash
   nc -v localhost 8080
   ```
**Criterio de Éxito:** Los registros (logs) del servidor deben mostrar una nueva conexión aceptada con un descriptor de archivo válido. El servidor debe permanecer completamente reactivo a un segundo comando `nc` ejecutado desde otra ventana del terminal.

### 🧠 Testing del Flujo del HTTP Parser
Para verificar de forma asilada que la Máquina de Estados Finita (FSM) fragmenta la red carácter a carácter sin pérdida de estado:

Crea un arnés de prueba local (en un `main` temporal) e inyecta un array de caracteres fraccionado:

```cpp
// Simulando la llegada asíncrona de datos:
parser.feed('G'); parser.feed('E'); parser.feed('T'); parser.feed(' ');
// (Pausa simulando delay de la red...)
parser.feed('/'); parser.feed('\r'); parser.feed('\n');
```
**Criterio de Éxito:** El parser debe permanecer grácilmente en los estados intermedios (`STATE_URI`, `STATE_VERSION`, etc.) durante la fragmentación temporal, y solo transicionar a `STATE_COMPLETE` cuando se hayan procesado al 100% los caracteres delimitadores del protocolo.

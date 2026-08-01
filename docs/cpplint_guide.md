# Guía de Uso de `cpplint` para el Proyecto Webserv

¡Hola Raúl! 

Como parte de las mejoras en nuestra integración continua (CI/CD) y para asegurar que nuestro código tiene calidad profesional para la evaluación de 42, hemos integrado **`cpplint`**. Aquí tienes un resumen rápido de por qué lo usamos, cómo instalarlo y cómo debes usarlo en tu día a día.

## 1. ¿Por qué usamos `cpplint`?

`cpplint` es una herramienta desarrollada por Google para garantizar que el código C++ cumple estrictamente con la **Guía de Estilo de C++ de Google**. Lo usamos por las siguientes razones:

- **Estandarización**: Asegura que todo el equipo escribe el código con el mismo formato (nombres de variables, indentación, espacios, saltos de línea).
- **Legibilidad**: Un código uniforme es mucho más fácil de leer, revisar (pull requests) y evaluar.
- **Prevención de Errores**: Detecta malas prácticas comunes (como problemas con el orden de los `#include`, olvidos de macros `#ifndef` en cabeceras o líneas demasiado largas).
- **Requisito del CI/CD**: Actualmente, GitHub Actions bloqueará cualquier subida a la rama `develop` o `main` si el código no pasa el linter. ¡Pasarlo en local nos ahorra tiempo!

---

## 2. Cómo instalar `cpplint` en tu terminal

Dependiendo de tu sistema operativo, la instalación es muy sencilla. La forma más universal es usar `pip` (el gestor de paquetes de Python).

> [!TIP]
> **Instalación recomendada (Vía Python/Pip):**
> ```bash
> pip install cpplint
> # O si usas pip3:
> pip3 install cpplint
> ```

Si usas distribuciones Linux como Fedora (con `dnf`) o Ubuntu/Debian (con `apt`), puedes buscarlo en los repositorios de tu sistema:
```bash
# Ubuntu / Debian
sudo apt install cpplint

# Fedora (o usar pip si no está en el repo)
sudo dnf install cpplint
```

---

## 3. Instrucciones de Uso (El Flujo de Trabajo)

> [!IMPORTANT]
> **Regla de Oro:** Siempre debes ejecutar el linter **ANTES** de hacer `git commit` y `git push`.

Cada vez que crees código nuevo, modifiques una clase o arregles un bug, sigue estos pasos:

### Paso 1: Ejecutar el análisis
Abre tu terminal en la raíz del proyecto (`webserv-42`) y ejecuta el siguiente comando para analizar recursivamente todas las carpetas de código fuente y cabeceras:

```bash
cpplint --recursive src/ include/
```

### Paso 2: Leer el reporte
Si el código está perfecto, no verás errores. Si hay fallos de formato, verás una salida como esta:
```text
src/main.cpp:41:  Lines should be <= 80 characters long  [whitespace/line_length] [2]
include/handlers/FileHandler.hpp:2:  #ifndef header guard has wrong style  [build/header_guard] [5]
```
La salida te dice **el archivo, la línea exacta y el motivo del error**.

### Paso 3: Corregir y Verificar
1. Ve a tu editor (VSCode) a la línea indicada.
2. Corrige el fallo (borra el espacio sobrante, acorta la línea, etc.).
3. Vuelve a ejecutar `cpplint --recursive src/ include/`.
4. Repite hasta que el linter te devuelva **0 errores**.

> [!WARNING]
> Si añades un nuevo archivo `.hpp` o `.cpp`, asegúrate de añadir siempre el mensaje de copyright en la primera línea para que el linter no se queje:
> `// Copyright 2026 raperez- serjimen`

¡Y eso es todo! Con esto mantendremos el proyecto súper limpio y evitaremos bloqueos en GitHub Actions.

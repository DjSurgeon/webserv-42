# Guía de Uso de `clang-format` para el Proyecto Webserv

¡Hola de nuevo Raúl!

Además de `cpplint` (que vigila semántica, convenciones y cabeceras), hemos integrado una segunda herramienta vital para nuestro CI/CD: **`clang-format`**. 

Mientras que cpplint es un "linter", clang-format es un **"formateador estricto"**. Se encarga exclusivamente de la estética del código: dónde poner los espacios, cómo tabular, dónde poner las llaves `{}` y cómo romper líneas largas.

## 1. ¿Por qué usamos `clang-format`?

- **Unificación Estética**: El proyecto tiene un archivo oculto llamado `.clang-format` en la raíz. Este archivo contiene las reglas exactas de Google. Si todos usamos esta herramienta, el código de todo el equipo se verá exactamente igual, como si lo hubiera escrito una sola persona.
- **Automatización**: A diferencia de otros errores que hay que pensar cómo arreglarlos, los errores de formato se pueden **arreglar de forma 100% automática** en menos de un segundo.
- **CI/CD Implacable**: GitHub Actions rechazará cualquier pull request que tenga un solo espacio fuera de lugar.

---

## 2. Cómo instalar `clang-format`

A diferencia de cpplint, clang-format es una herramienta compilada (parte del ecosistema LLVM/Clang) y se suele instalar desde el gestor de paquetes del sistema operativo.

> [!TIP]
> **Instalación según tu sistema operativo:**
> ```bash
> # Fedora (La que usamos nosotros normalmente)
> sudo dnf install clang-tools-extra
> 
> # Ubuntu / Debian
> sudo apt install clang-format
> 
> # macOS (usando Homebrew)
> brew install clang-format
> ```

---

## 3. Configuración del proyecto (`.clang-format`)

Para que la herramienta sepa qué estilo aplicar, busca un archivo llamado `.clang-format` en la raíz del repositorio. Si por algún casual clonas el repositorio y no lo tienes o se borra, crea un archivo llamado `.clang-format` justo en la carpeta principal de `webserv-42` y ponle exactamente este contenido (son nuestras reglas basadas en Google Style):

```yaml
BasedOnStyle: Google
IndentWidth: 2
TabWidth: 2
UseTab: Never
ColumnLimit: 80
AllowShortFunctionsOnASingleLine: Empty
DerivePointerAlignment: false
PointerAlignment: Left
SpaceBeforeParens: ControlStatements
SortIncludes: true
IncludeBlocks: Regroup
```

---

## 4. Instrucciones de Uso (El Flujo de Trabajo)

Aquí te explico las dos formas de usarlo.

### Opción A: Modo "Chivato" (Solo ver los errores)
Si quieres comprobar si tu código cumple las reglas **sin modificar nada**, ejecuta el comando en modo `--dry-run`. Esto es exactamente lo que hace GitHub Actions internamente.

```bash
find src include -type f \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-format --dry-run --Werror
```
Si hay errores, te mostrará la línea y un pequeño cursor `^` indicando dónde falta o sobra un espacio/salto de línea. Si no sale nada, ¡está perfecto!

### Opción B: Modo "Mágico" (Arreglarlo todo automáticamente)
Esta es la opción que vas a usar el 99% de las veces. Le dice a clang-format que edite los archivos y aplique el formato correcto. Fíjate en la bandera `-i` (in-place).

> [!IMPORTANT]
> **Comando de Auto-Formateo (Ejecutar antes de cada commit):**
> ```bash
> find src include -type f \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-format -i
> ```

### El Flujo de Trabajo Ideal
1. Programas tu código.
2. Ejecutas el auto-formateo mágico: `find src include -type f \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-format -i`
3. Opcional: Lanzas un `make test` para asegurar que todo compila y pasa tras el formateo.
4. Haces `git commit` y `git push`.

¡Con este comando y el de cpplint, nuestro código será el más limpio de todo 42!

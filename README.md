# rat

`rat` significa **recursive cat**.

Es una herramienta de línea de comandos escrita en C cuyo objetivo principal es facilitar la copia del contenido completo de un proyecto para compartirlo con un LLM. Genera una salida lineal que incluye la estructura de directorios, los archivos procesados y los elementos omitidos, diseñada para copiarse fácilmente al portapapeles (por ejemplo, mediante `| wl-copy`) y pegarse directamente en una conversación con modelos de IA como ChatGPT. Resulta especialmente útil para quienes no disponen de agentes integrados en su editor o necesitan proporcionar a la IA una visión completa del contexto del proyecto.

## Qué hace

`rat` está diseñada para exportar el contenido de un proyecto de forma lineal, mostrando directorios, archivos y los elementos que se omiten.

- Recorre directorios recursivamente
- Imprime archivos regulares
- Ignora directorios pesados comunes como `.git`, `node_modules`, `build`, `dist`, `.next`, `__pycache__`
- Ignora extensiones binarios o multimedia como `.jpg`, `.png`, `.pdf`, `.zip`, `.so`, etc.
- No sigue symlinks como directorios
- Omite archivos mayores a 1 MB
- Genera salida simple y copiable

## Ejemplo de uso

```bash
gcc main.c -o rat
./rat .
./rat test
./rat --version
./rat -di
./rat -ei
./rat . -i vendor
```

## Salida esperada

El programa imprime información sobre el directorio y los archivos procesados:

- `[DIR]` para cada directorio entrado
- `[SKIPPED DIR]` para directorios ignorados
- `[SKIPPED BY EXT]` para archivos omitidos por extensión
- `[FILE TOO LARGE]` para archivos mayores a 1 MB
- `=== FILE: ... ===` seguido del contenido del archivo

## Opciones

- `-h`, `--help`: muestra ayuda
- `--version`: muestra la versión
- `-di`: lista los directorios ignorados
- `-ei`: lista las extensiones ignoradas
- `-i <nombre_dir>`: añade un directorio extra a ignorar en esta ejecución

## Directorios ignorados

Actualmente `rat` ignora:

- `.git`
- `.venv`
- `node_modules`
- `build`
- `.next`
- `dist`
- `__pycache__`

## Extensiones ignoradas

Actualmente `rat` ignora:

- `.jpg`
- `.jpeg`
- `.png`
- `.gif`
- `.pdf`
- `.zip`
- `.tar`
- `.rar`
- `.gz`
- `.exe`
- `.bin`
- `.so`
- `.dll`
- `.dylib`
- `.class`
- `.o`
- `.hex`
- `.iso`

## Límite de tamaño

`rat` omite archivos cuyo tamaño supera `1000 KB` (`1 MB`).

## Implementación

El código usa APIs clásicas de Unix/Linux:

- `opendir()` / `readdir()` / `closedir()`
- `lstat()` para información de archivos
- `fopen()` / `fread()` / `fwrite()` para leer archivos
- construcción manual de rutas con `join_path()`

## Estructura del proyecto

- `main.c` – punto de entrada, parsing de argumentos y recorrido principal
- `README-rat.md` – documentación extendida del proyecto
- `LICENSE` – licencia del proyecto
- `test/` – carpeta con pruebas y ejemplos

## Compilación

```bash
gcc main.c -o rat
```

## Uso

```bash
./rat [directorio]
```

Si no se especifica directorio, `rat` usa el directorio actual.

## Motivación

`rat` busca facilitar la extracción de contexto de un proyecto para revisarlo manualmente o para alimentar a un LLM sin tener que abrir cada archivo por separado.

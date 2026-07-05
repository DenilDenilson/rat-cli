<h1 align="center">rat</h1>

<p align="center">
  <strong>Recursive cat para convertir un directorio en contexto listo para pegar en un LLM.</strong>
</p>

<p align="center">
  <img alt="Version" src="https://img.shields.io/badge/version-0.8.0-16a34a?style=flat-square">
  <img alt="Language" src="https://img.shields.io/badge/C-111?style=flat-square&logo=c">
  <img alt="Platform" src="https://img.shields.io/badge/Unix%20CLI-111?style=flat-square">
  <img alt="Use case" src="https://img.shields.io/badge/contexto%20para-LLMs-0891b2?style=flat-square">
  <img alt="Output" src="https://img.shields.io/badge/salida-copiable-7c3aed?style=flat-square">
</p>

<p align="center">
  <a href="#-resumen">Resumen</a> ·
  <a href="#-problema">Problema</a> ·
  <a href="#-demo-de-salida">Demo</a> ·
  <a href="#-qué-hace">Qué hace</a> ·
  <a href="#-opciones">Opciones</a> ·
  <a href="#-compilación-y-uso">Compilación</a>
</p>

---

## 🧭 Resumen

`rat` significa **recursive cat**.

Es una herramienta de línea de comandos escrita en C para convertir un proyecto
en una salida lineal, copiable y útil para modelos de lenguaje. Recorre un
directorio, imprime la estructura y vuelca el contenido de los archivos que
normalmente sí necesita un LLM para entender el contexto.

La idea central es sencilla: **darle a ChatGPT u otro LLM el contexto de un
proyecto sin tener que copiar archivo por archivo**.

## 🚨 Problema

Al trabajar con proyectos completos y LLMs:

| Dolor | Consecuencia |
|---|---|
| 🧠 Los agentes en IDEs como Codex consumen contexto rápido | La sesión puede pausarse o quedarse sin margen para seguir trabajando |
| 📦 Un proyecto tiene muchos archivos repartidos | El chat no entiende la relación entre rutas, estructura y código |
| 🧾 Copiar archivos uno por uno es tedioso | Se pierde tiempo y es fácil olvidar piezas importantes del contexto |
| 🗑️ No todo el repositorio sirve para el prompt | Builds, dependencias, binarios y assets gastan tokens sin aportar mucho |

`rat` prepara el contexto que el LLM necesita:

```txt
"Este es mi proyecto completo.
Revisa la estructura, entiende los archivos y ayúdame con el cambio."
```

El resultado es una **salida copiable** con estructura, rutas, contenido de
archivos relevantes y marcas de lo que se omitió. Así puedes usar chats como
ChatGPT para razonar sobre más contexto sin depender siempre del presupuesto de
contexto de un agente dentro del IDE.

`rat` manda todo el texto a la consola mediante `stdout`. Para dejar ese
contexto listo para pegar, normalmente se combina con una herramienta de
portapapeles:

```bash
./rat . | wl-copy
```

Esto es intencional: `rat` se mantiene granular y solo se encarga de producir la
salida. El usuario decide si quiere verla en terminal, redirigirla a un archivo,
pasarla a `wl-copy` o usar cualquier otra herramienta del sistema.

## 📦 Instalación

### Arch Linux

Descarga el paquete `.pkg.tar.zst` desde la última release de GitHub y luego instálalo con `pacman`:

```bash
curl -LO https://github.com/DenilDenilson/rat-llm/releases/download/v0.8.1/rat-llm-git-0.8.1.r0.gde2e4cd-1-x86_64.pkg.tar.zst
sudo pacman -U ./rat-llm-git-0.8.1.r0.gde2e4cd-1-x86_64.pkg.tar.zst
```

Luego verifica la instalación:

```bash
rat-llm --version
rat-llm -h
```

### Compilar desde código fuente

Si prefieres compilarlo manualmente:

```bash
git clone https://github.com/DenilDenilson/rat-llm.git
cd rat-llm
gcc -O2 -Wall -Wextra -pedantic src/main.c -o rat-llm
```

Para usarlo de forma local:

```bash
./rat-llm --version
./rat-llm .
```

Si quieres instalar el binario manualmente en tu sistema:

```bash
sudo install -Dm755 rat-llm /usr/local/bin/rat-llm
```

Y luego:

```bash
rat-llm --version
```

### Instalación futura desde AUR

Más adelante, cuando el paquete esté publicado en AUR, podrá instalarse con un helper como `yay`:

```bash
yay -S rat-llm-git
```


## 🖥️ Demo de salida

Cuando se ejecuta sobre `test/`, la herramienta produce una salida como esta:

```text
rat: Procesando ruta test


[DIR] test/raw
=== FILE: test/raw/ignore.c ===
// Esto debe ignorarse

[SKIPPED NO EXT] test/Makefile


[DIR] test/assets
[SKIPPED BY EXT] test/assets/file.pdf

[SKIPPED BY EXT] test/assets/image.jpg

[SKIPPED NO EXT] test/README


[DIR] test/src
=== FILE: test/src/utils.c ===
helper


=== FILE: test/src/main.c ===
#include <stdio.h>
int main(){return 0;}


[SKIPPED DIR] test/node_modules
[SKIPPED DIR] test/build
```

## 🧪 Qué hace

- Recorre directorios recursivamente
- Imprime archivos regulares
- Ignora directorios pesados comunes como `.git`, `node_modules`, `build`,
  `dist`, `.next` y `__pycache__`
- Ignora extensiones binarias o multimedia como `.jpg`, `.png`, `.pdf`, `.zip`
  o `.so`
- Ignora archivos sin extensión para evitar volcar binarios como ejecutables
- No sigue symlinks como directorios
- Omite archivos mayores a `1000 KB`
- Permite ignorar directorios extra por nombre (`logs`) o ruta (`data/evidence/`)
- Lee `.ratignore` desde la raíz procesada para omitir rutas persistentes
- Genera una salida simple y copiable

## ⚙️ Opciones

- `-h`, `--help`: muestra ayuda
- `--version`: muestra la versión
- `-di`: lista los directorios ignorados
- `-ei`: lista las extensiones ignoradas
- `-i <nombre_o_ruta_dir...>`: agrega uno o más directorios extra a ignorar por nombre o ruta en esta ejecución

## 🚫 Extensiones ignoradas y filtros

Para evitar gastar tokens en ruido, `rat` omite por defecto rutas y archivos que
normalmente no ayudan al LLM a entender el proyecto.

| Tipo | Valores |
| ---- | ------- |
| Directorios ignorados | `.git`, `.venv`, `node_modules`, `build`, `.next`, `dist`, `.astro`, `.cache`, `__pycache__` |
| Extensiones ignoradas | `.jpg`, `.jpeg`, `.png`, `.gif`, `.pdf`, `.zip`, `.tar`, `.rar`, `.gz`, `.exe`, `.bin`, `.so`, `.dll`, `.dylib`, `.class`, `.o`, `.hex`, `.iso` |
| Archivos sin extensión | Se omiten para evitar imprimir binarios como ejecutables compilados |
| Tamaño máximo | `1000 KB` por archivo |
| Symlinks | Se reportan como `[SYMLINK]`, pero no se siguen como directorios |

## 📁 Estructura

- `main.c` - entrada principal, parsing de argumentos y recorrido recursivo
- `test/` - carpeta con pruebas y ejemplos
- `README.md` - documentación breve del proyecto
- `LICENSE` - licencia del proyecto

## ⚠️ Advertencia sobre el nombre

Puede existir un choque de nombre con otro proyecto llamado `rat`, un `cat`
escrito en Rust. Por eso, aunque este repositorio actualmente compila el binario
como `rat`, se está evaluando usar `rat-cli` como nombre de salida para evitar
confusiones en sistemas donde ya exista otro comando con ese nombre.

## 🛠️ Compilación y uso

```bash
gcc main.c -o rat
./rat .
./rat test
./rat --version
./rat -di
./rat -ei
./rat . -i vendor cache tmp
./rat . -i data/back-matches/ data/evidence/ logs
./rat . | wl-copy
```

O, si quieres evitar el posible choque de nombre:

```bash
gcc main.c -o rat-cli
./rat-cli .
./rat-cli . | wl-copy
```

Si no se especifica directorio, `rat` usa el directorio actual.

## 📝 `.ratignore`

Si existe un archivo `.ratignore` en el directorio que procesas, `rat` lo lee
antes de recorrer el árbol. Cada línea representa un patrón a omitir.

```text
# comentarios y líneas vacías se ignoran
logs/
data/back-matches/
data/evidence/
src/secret.c
src/*.tmp
```

Los patrones se comparan contra rutas relativas al directorio procesado. Por
ejemplo, `main.c` solo omite `./main.c`; para omitir `src/main.c` debes escribir
`src/main.c`. También puedes usar patrones simples como `src/*.tmp`. Las rutas
de directorio aceptan `/` final, que es la forma natural de escribirlas en Linux.

## 🔍 Cómo funciona

Internamente usa APIs clásicas de Unix/Linux:

- `opendir()` / `readdir()` / `closedir()`
- `lstat()` para detectar tipos de archivo
- `fopen()` / `fread()` / `fwrite()` para volcar contenido
- construcción manual de rutas con `join_path()`

## 💡 Motivación

`rat` existe para que el trabajo con LLMs no dependa de copiar archivos a mano.
El objetivo es entregar una vista completa, copiable y útil: suficiente
estructura para orientarse, suficiente contenido para razonar y suficientes
omisiones para no desperdiciar tokens en material irrelevante.

## 📄 Licencia

MIT License. Ver [LICENSE](/home/dnl/Projects/rat/LICENSE).

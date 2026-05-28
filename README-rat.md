# rat

**rat** significa **recursive cat**.

Es una herramienta CLI escrita en C para recorrer directorios recursivamente e imprimir archivos en un formato limpio y útil para humanos y LLMs.

La idea es mezclar conceptos de:

- `ls`
- `cat`
- recursión tipo `tree`

pero con un output pensado para exportar contexto de un proyecto.

---

## Qué hace

`rat` actualmente:

- recorre directorios recursivamente
- imprime archivos regulares
- ignora directorios pesados como `.git`, `node_modules`, `build`, `dist`, `.next`, `__pycache__`
- ignora ciertas extensiones no deseadas como `.jpg`, `.png`, `.pdf`, `.zip`, `.so`, etc.
- evita imprimir archivos demasiado grandes
- no sigue symlinks como si fueran directorios
- imprime una salida simple y copiable

---

## Ejemplo de salida

```text
rat: Procesando ruta Projects/rat/

[SKIPPED DIR] Projects/rat/.git
[DIR] Projects/rat/test
=== FILE: Projects/rat/test/Makefile ===
Makefile content

[DIR] Projects/rat/test/assets
=== FILE: Projects/rat/test/assets/file.pdf ===
[SKIPPED BY EXT] Projects/rat/test/assets/file.pdf

=== FILE: Projects/rat/test/assets/image.jpg ===
[SKIPPED BY EXT] Projects/rat/test/assets/image.jpg

=== FILE: Projects/rat/test/README ===
README content
```

---

## Motivación

Copiar archivo por archivo de un proyecto para dárselo a un LLM es tedioso.

`rat` busca resolver eso generando una salida única, lineal y estructurada, donde puedas ver:

- qué directorios fueron recorridos
- qué directorios fueron ignorados
- qué archivos se imprimieron
- qué archivos fueron omitidos por tamaño o extensión

---

## Estado actual

La versión actual implementa:

- recorrido recursivo con `opendir`, `readdir`, `closedir`
- clasificación con `lstat`
- construcción manual de rutas
- lectura de archivos con `fread`
- filtros por nombre de directorio
- filtros por extensión
- límite de tamaño por archivo
- flag `--version`

Versión actual:

```text
rat v0.1.0
```

---

## Compilación

```bash
gcc main.c -o rat
```

---

## Instalación

### Instalación local de usuario

Si tu `PATH` incluye `~/.local/bin`:

```bash
cp rat ~/.local/bin/
```

### Instalación global

```bash
sudo cp rat /usr/local/bin/
```

Luego podrás usarlo desde cualquier directorio:

```bash
rat .
rat src
rat --version
```

---

## Uso

```bash
rat [directorio]
```

### Ejemplos

```bash
rat .
rat test
rat Projects/rat
```

### Versión

```bash
rat --version
```

---

## Comportamiento actual

### Directorios ignorados

Actualmente `rat` ignora estos directorios:

- `.git`
- `node_modules`
- `build`
- `.next`
- `dist`
- `__pycache__`

### Extensiones ignoradas

Actualmente `rat` ignora estas extensiones:

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

### Tamaño máximo

Por defecto:

- `1000 KB` (`1 MB`) por archivo

Si un archivo supera ese límite, no se imprime su contenido.

---

## Diseño técnico

`rat` está implementado en C y usa interfaces clásicas de Unix/Linux:

- `opendir()`
- `readdir()`
- `closedir()`
- `lstat()`
- `fopen()`
- `fread()`
- `fwrite()`

### Flujo general

1. abrir directorio
2. iterar entradas
3. ignorar `.` y `..`
4. construir ruta completa
5. consultar metadata con `lstat`
6. decidir si es:
   - directorio
   - archivo regular
   - symlink
   - otro
7. aplicar filtros
8. imprimir contenido si corresponde

---

## Decisiones de diseño

### `lstat` en vez de `stat`

Se usa `lstat` para no seguir symlinks automáticamente y evitar problemas de recursión o loops.

### `readdir` en vez de `scandir`

Se usa `readdir` porque permite recorrer por niveles sin cargar un directorio completo en memoria de una sola vez.

### `fread` en vez de `fgetc`

Se usa lectura por bloques para reducir overhead y hacer la herramienta más eficiente al imprimir archivos completos.

### filtros por extensión

En esta etapa se prefirió una política explícita por extensión en vez de heurísticas de binario, por simplicidad y control.

---

## Limitaciones actuales

Esta versión todavía tiene varias mejoras pendientes:

- no tiene `--help`
- no tiene flags configurables para excluir/incluir
- no imprime rutas relativas al root todavía
- no ordena el output
- todavía vive en un solo archivo fuente
- no tiene tests automatizados
- no tiene `Makefile`
- no soporta múltiples paths de entrada

---

## Roadmap

Ideas para próximas versiones:

- [ ] agregar `--help`
- [ ] modularizar en varios archivos (`main.c`, `walk.c`, `filters.c`, etc.)
- [ ] imprimir rutas relativas al root en vez de rutas completas
- [ ] permitir configurar extensiones/directorios ignorados
- [ ] permitir configurar `max file size`
- [ ] agregar `Makefile`
- [ ] ordenar salida alfabéticamente
- [ ] empaquetar para Arch con `PKGBUILD`

---

## Estructura futura propuesta

```text
rat/
├── src/
│   ├── main.c
│   ├── walk.c
│   ├── walk.h
│   ├── path.c
│   ├── path.h
│   ├── filters.c
│   ├── filters.h
│   ├── output.c
│   └── output.h
├── README.md
├── LICENSE
└── Makefile
```

---

## Licencia

MIT

Ver archivo `LICENSE`.

---

## Autor

Denilson

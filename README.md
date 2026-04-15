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
Responsabilidad de cada uno
main.c
argumentos, --help, punto de entrada
walk.c
list_directory() y recursión
path.c
join_path()
filters.c
should_skip_dir(), should_skip_file(), get_extension()
output.c
print_file() y luego helpers de formato
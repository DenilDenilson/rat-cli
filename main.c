#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_FILE_SIZE (1000 * 1024)
#define RAT_VERSION   "0.3.0"

const char *ignored_dirs[] = {
    ".git",
    ".venv",
    "node_modules",
    "build",
    ".next",
    "dist",
    "__pycache__",
    NULL
};

const char *ignored_exts[] = {
    ".jpg", ".jpeg", ".png", ".gif",
    ".pdf", ".zip", ".tar", ".rar",
    ".gz", ".exe", ".bin", ".so",
    ".dll", ".dylib", ".class", ".o",
    ".hex", ".iso",
    NULL
};

void print_string_list(const char *title, const char *items[]) {
    printf("%s\n", title);

    for (int i = 0; items[i] != NULL; i++) {
        printf("  %s\n", items[i]);
    }
}

void print_help(const char *prog_name) {
    printf("uso: %s [directorio] [-i nombre_dir]\n", prog_name);
    printf("\n");
    printf("Herramienta CLI para recorrer directorios recursivamente e imprimir archivos.\n");
    printf("\n");
    printf("Opciones:\n");
    printf("  -h, --help       muestra esta ayuda\n");
    printf("  --version        muestra la versión\n");
    printf("  -di              lista los directorios ignorados\n");
    printf("  -ei              lista las extensiones ignoradas\n");
    printf("  -i nombre_dir    ignora además un directorio por nombre en esta ejecución\n");
}

int should_skip_dir(const char *name, const char *extra_ignored_dir) {
    if (name == NULL) return 1;

    if (extra_ignored_dir != NULL && strcmp(name, extra_ignored_dir) == 0) {
        return 1;
    }

    for (int i = 0; ignored_dirs[i] != NULL; i++) {
        if (strcmp(name, ignored_dirs[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

const char *get_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name) return NULL;

    return dot;
}

int should_skip_file(const char *name) {
    const char *ext = get_extension(name);
    if (!ext) return 0;

    for (int i = 0; ignored_exts[i] != NULL; i++) {
        if (strcasecmp(ext, ignored_exts[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

void print_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "rat: no se pudo abrir archivo '%s': '%s'\n", path, strerror(errno));
        return;
    }

    unsigned char buffer[1024];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        fwrite(buffer, 1, n, stdout);
    }

    fclose(f);
}

int join_path(char *buf, size_t bufsize, const char *base, const char *name) {
    size_t base_len = strlen(base);
    size_t name_len = strlen(name);

    int needs_slash = (base_len > 0 && base[base_len - 1] != '/');

    size_t total = base_len + needs_slash + name_len + 1;
    if (total > bufsize) return -1;

    memcpy(buf, base, base_len);

    size_t pos = base_len;
    if (needs_slash) buf[pos++] = '/';

    memcpy(buf + pos, name, name_len);
    buf[pos + name_len] = '\0';

    return 0;
}

int list_directory(const char *path, const char *extra_ignored_dir) {
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "rat: no se pudo abrir '%s': '%s'\n", path, strerror(errno));
        return -1;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (join_path(fullpath, sizeof(fullpath), path, entry->d_name) != 0) {
            fprintf(stderr, "rat: ruta muy larga: '%s/%s'\n", path, entry->d_name);
            continue;
        }

        struct stat st;
        if (lstat(fullpath, &st) != 0) {
            fprintf(stderr, "rat: lstat falló en '%s': '%s'\n", fullpath, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (should_skip_dir(entry->d_name, extra_ignored_dir)) {
                printf("[SKIPPED DIR] %s\n", fullpath);
                continue;
            }

            printf("[DIR] %s\n", fullpath);
            list_directory(fullpath, extra_ignored_dir);
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            printf("[SYMLINK] %s\n", fullpath);
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            if ((long long)st.st_size > MAX_FILE_SIZE) {
                printf("[FILE TOO LARGE] %s (%lld bytes)\n\n", fullpath, (long long)st.st_size);
                continue;
            }

            if (should_skip_file(entry->d_name)) {
                printf("[SKIPPED BY EXT] %s\n\n", fullpath);
                continue;
            }

            printf("=== FILE: %s ===\n", fullpath);
            print_file(fullpath);
            printf("\n\n");
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = ".";
    const char *extra_ignored_dir = NULL;

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("rat v%s\n", RAT_VERSION);
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-di") == 0) {
        print_string_list("Directorios ignorados:", ignored_dirs);
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-ei") == 0) {
        print_string_list("Extensiones ignoradas:", ignored_exts);
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "-i") == 0) {
        extra_ignored_dir = argv[2];
    } else if (argc == 4 && strcmp(argv[2], "-i") == 0) {
        path = argv[1];
        extra_ignored_dir = argv[3];
    } else if (argc == 2) {
        path = argv[1];
    } else if (argc != 1) {
        fprintf(stderr, "uso: %s [directorio] [-i nombre_dir]\n", argv[0]);
        return 1;
    }

    printf("rat: Procesando ruta %s\n\n\n", path);

    if (list_directory(path, extra_ignored_dir) != 0) {
        return 1;
    }

    return 0;
}
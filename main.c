#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/limits.h>

#define MAX_FILE_SIZE   (1000 * 1024) // 1000 KB o 1 MB

const char *ignored_dirs[] = {
    ".git",
    "node_modules",
    "build",
    ".next",
    "dist",
    "__pycache__",
    NULL
};

int should_skip_dir(const char *name) {
    if (name == NULL) return 1;

    for (int i = 0; ignored_dirs[i] != NULL; i++) {
        if (strcmp(name, ignored_dirs[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

const char *ignored_exts[] = {
    ".jpg", ".jpeg", ".png", ".gif",
    ".pdf", ".zip", ".tar", ".rar",
    ".gz", ".exe", ".bin", ".so",
    NULL
};

const char* get_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name) return NULL;

    return dot;
}

int should_skip_file(const char *name) {
    const char *ext = get_extension(name);
    if (!ext) return 0; // Algunos files sin extensión como MAKEFILE se necesitan leer

    for (int i = 0; ignored_exts[i] != NULL; i++) {
        if (strcasecmp(ext, ignored_exts[i]) == 0) {
            return 1; // debe ignorar
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

    size_t total = base_len + needs_slash + name_len + 1; // +1 para el \0
    if (total > bufsize) return 1;

    memcpy(buf, base, base_len);

    size_t pos = base_len;
    if (needs_slash) buf[pos++] = '/';

    memcpy(buf + pos, name, name_len);
    buf[pos + name_len] = '\0';

    return 0;
}

static const char *file_type_string(mode_t mode) {
    if (S_ISREG(mode))  return "file";
    if (S_ISDIR(mode))  return "dir";
    if (S_ISLNK(mode))  return "symlink";
    return "other";
}


int list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "rat: No se pudo abrir '%s': '%s'\n", path, strerror(errno));

        return -1;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        if (join_path(fullpath, sizeof(fullpath), path, entry->d_name) != 0) {
            fprintf(stderr, "rat: ruta muy larga\n");

            continue;
        }

        struct stat st;
        if (lstat(fullpath, &st) != 0) {
            fprintf(stderr, "rat: lstat falló en '%s': '%s'\n", fullpath, strerror(errno));
            continue;
        }

        // printf("[%s] %s\n", file_type_string(st.st_mode), fullpath);

        if (S_ISDIR(st.st_mode)) {
            if (should_skip_dir(entry->d_name)) {
                printf("[SKIPPED DIR] %s\n", fullpath);
                continue;
            }
            
            printf("[DIR] %s\n", fullpath);
            list_directory(fullpath);
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            printf("=== FILE: %s ===\n", fullpath);
            
            // Que no sea mayor a 1000 KB
            if ((long long) st.st_size > MAX_FILE_SIZE) {
                printf("[FILE TOO LARGE: %lld bytes]\n\n", (long long)st.st_size);
                continue;
            }
            
            // Ignoramos formatos que no nos interesan
            if (should_skip_file(entry->d_name)) {
                printf("[SKIPPED BY EXT] %s\n\n", fullpath);
                continue;
            }

            // Imprime el archivo
            print_file(fullpath);
            printf("\n\n");
        }
    }

    closedir(dir);

    return 0;
}

int main(int argc, char **argv) {
    const char *path = ".";
    
    if (argc > 2) {
        fprintf(stderr, "uso: %s [directorio]\n", argv[0]);

        return 1; // Esto es un error
    }

    if (argc == 2) {
        path = argv[1];
    }

    printf("rat: Procesando ruta %s\n\n\n", path);

    if (list_directory(path) != 0) return 1;

    return 0;
}
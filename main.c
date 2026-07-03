#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <fnmatch.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_FILE_SIZE (1000 * 1024)
#define RAT_VERSION   "0.8.0"
#define MAX_RATIGNORE_PATTERNS 256

const char *ignored_dirs[] = {
    ".git",
    ".venv",
    "node_modules",
    "build",
    ".next",
    "dist",
    ".astro",
    ".cache",
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

typedef struct {
    char pattern[PATH_MAX];
    int dir_only;
} RatignorePattern;

typedef struct {
    RatignorePattern items[MAX_RATIGNORE_PATTERNS];
    int count;
} Ratignore;

int join_path(char *buf, size_t bufsize, const char *base, const char *name);

void print_string_list(const char *title, const char *items[]) {
    printf("%s\n", title);

    for (int i = 0; items[i] != NULL; i++) {
        printf("  %s\n", items[i]);
    }
}

void print_help(const char *prog_name) {
    printf("uso: %s [directorio] [-i nombre_o_ruta_dir...]\n", prog_name);
    printf("\n");
    printf("Herramienta CLI para recorrer directorios recursivamente e imprimir archivos.\n");
    printf("Lee .ratignore desde el directorio procesado si existe.\n");
    printf("\n");
    printf("Opciones:\n");
    printf("  -h, --help       muestra esta ayuda\n");
    printf("  --version        muestra la versión\n");
    printf("  -di              lista los directorios ignorados\n");
    printf("  -ei              lista las extensiones ignoradas\n");
    printf("  -i nombre_o_ruta_dir... ignora uno o más directorios por nombre o ruta en esta ejecución\n");
}

char *trim_whitespace(char *s) {
    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return s;
    }

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return s;
}

void normalize_dir_ref(const char *src, char *dst, size_t dst_size) {
    if (dst_size == 0) return;

    while (src[0] == '.' && src[1] == '/') {
        src += 2;
    }

    size_t pos = 0;
    while (src[pos] != '\0' && pos + 1 < dst_size) {
        dst[pos] = src[pos];
        pos++;
    }

    dst[pos] = '\0';

    while (pos > 0 && dst[pos - 1] == '/') {
        dst[pos - 1] = '\0';
        pos--;
    }
}

void load_ratignore(const char *root_path, Ratignore *ratignore) {
    ratignore->count = 0;

    char ratignore_path[PATH_MAX];
    if (join_path(ratignore_path, sizeof(ratignore_path), root_path, ".ratignore") != 0) {
        fprintf(stderr, "rat: ruta muy larga para .ratignore en '%s'\n", root_path);
        return;
    }

    FILE *f = fopen(ratignore_path, "r");
    if (!f) {
        return;
    }

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), f) != NULL) {
        char *pattern = trim_whitespace(line);

        if (pattern[0] == '\0' || pattern[0] == '#') {
            continue;
        }

        size_t pattern_len = strlen(pattern);
        ratignore->items[ratignore->count].dir_only = pattern_len > 0 && pattern[pattern_len - 1] == '/';
        normalize_dir_ref(pattern, ratignore->items[ratignore->count].pattern, PATH_MAX);

        if (ratignore->items[ratignore->count].pattern[0] == '\0') {
            continue;
        }

        ratignore->count++;
        if (ratignore->count >= MAX_RATIGNORE_PATTERNS) {
            fprintf(stderr, "rat: .ratignore superó el máximo de %d patrones\n", MAX_RATIGNORE_PATTERNS);
            break;
        }
    }

    fclose(f);
}

int path_matches_dir_ref(const char *path, const char *dir_ref) {
    size_t path_len = strlen(path);
    size_t ref_len = strlen(dir_ref);

    if (ref_len == 0) return 0;

    if (strcmp(path, dir_ref) == 0) {
        return 1;
    }

    return path_len > ref_len
        && path[path_len - ref_len - 1] == '/'
        && strcmp(path + path_len - ref_len, dir_ref) == 0;
}

int path_matches_ignore_pattern(const char *name, const char *fullpath, const char *pattern) {
    char normalized_fullpath[PATH_MAX];
    normalize_dir_ref(fullpath, normalized_fullpath, sizeof(normalized_fullpath));

    if (strchr(pattern, '/') == NULL) {
        return strcmp(name, pattern) == 0;
    }

    return path_matches_dir_ref(normalized_fullpath, pattern);
}

int should_skip_by_ratignore(const char *relative_path, int is_dir, const Ratignore *ratignore) {
    for (int i = 0; i < ratignore->count; i++) {
        RatignorePattern item = ratignore->items[i];

        if (item.dir_only && !is_dir) {
            continue;
        }

        if (fnmatch(item.pattern, relative_path, FNM_PATHNAME) == 0) {
            return 1;
        }
    }

    return 0;
}

int should_skip_dir(const char *name, const char *fullpath, char **extra_ignored_dirs, int extra_ignored_count) {
    if (name == NULL) return 1;

    for (int i = 0; i < extra_ignored_count; i++) {
        char normalized_ignored_dir[PATH_MAX];
        normalize_dir_ref(extra_ignored_dirs[i], normalized_ignored_dir, sizeof(normalized_ignored_dir));

        if (path_matches_ignore_pattern(name, fullpath, normalized_ignored_dir)) {
            return 1;
        }
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

int should_skip_file_without_extension(const char *name) {
    return get_extension(name) == NULL;
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

int list_directory(const char *path, const char *relative_path, char **extra_ignored_dirs, int extra_ignored_count, const Ratignore *ratignore) {
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

        char child_relative_path[PATH_MAX];
        if (join_path(child_relative_path, sizeof(child_relative_path), relative_path, entry->d_name) != 0) {
            fprintf(stderr, "rat: ruta relativa muy larga: '%s/%s'\n", relative_path, entry->d_name);
            continue;
        }

        struct stat st;
        if (lstat(fullpath, &st) != 0) {
            fprintf(stderr, "rat: lstat falló en '%s': '%s'\n", fullpath, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (should_skip_by_ratignore(child_relative_path, 1, ratignore)) {
                printf("[SKIPPED RATIGNORE DIR] %s\n", fullpath);
                continue;
            }

            if (should_skip_dir(entry->d_name, fullpath, extra_ignored_dirs, extra_ignored_count)) {
                printf("[SKIPPED DIR] %s\n", fullpath);
                continue;
            }

            printf("[DIR] %s\n", fullpath);
            list_directory(fullpath, child_relative_path, extra_ignored_dirs, extra_ignored_count, ratignore);
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            printf("[SYMLINK] %s\n", fullpath);
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            if (should_skip_by_ratignore(child_relative_path, 0, ratignore)) {
                printf("[SKIPPED RATIGNORE FILE] %s\n\n", fullpath);
                continue;
            }

            if ((long long)st.st_size > MAX_FILE_SIZE) {
                printf("[FILE TOO LARGE] %s (%lld bytes)\n\n", fullpath, (long long)st.st_size);
                continue;
            }

            if (should_skip_file_without_extension(entry->d_name)) {
                printf("[SKIPPED NO EXT] %s\n\n", fullpath);
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
    char **extra_ignored_dirs = NULL;
    int extra_ignored_count = 0;
    Ratignore ratignore;

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

    if (argc == 2 && strcmp(argv[1], "-i") == 0) {
        fprintf(stderr, "uso: %s [directorio] [-i nombre_o_ruta_dir...]\n", argv[0]);
        return 1;
    }

    if (argc >= 3 && strcmp(argv[1], "-i") == 0) {
        extra_ignored_dirs = &argv[2];
        extra_ignored_count = argc - 2;
    } else if (argc >= 4 && strcmp(argv[2], "-i") == 0) {
        path = argv[1];
        extra_ignored_dirs = &argv[3];
        extra_ignored_count = argc - 3;
    } else if (argc == 2) {
        path = argv[1];
    } else if (argc != 1) {
        fprintf(stderr, "uso: %s [directorio] [-i nombre_o_ruta_dir...]\n", argv[0]);
        return 1;
    }

    printf("rat: Procesando ruta %s\n\n\n", path);

    load_ratignore(path, &ratignore);

    if (list_directory(path, "", extra_ignored_dirs, extra_ignored_count, &ratignore) != 0) {
        return 1;
    }

    return 0;
}

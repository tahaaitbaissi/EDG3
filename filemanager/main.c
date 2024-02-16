#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// typedef struct {
//     char *name;
//     int len;
// } File;

typedef struct {
    char **items;
    int len;
} Files;

#define FILES_INIT {NULL, 0}


void FilesAppend(Files *files, const char *s)
{
    // if (files->items == NULL) {
    //     new = malloc((files->len + 1) * sizeof(char *));
    // } else {
    //     new = realloc(files->items, (files->len + 1) * sizeof(char *));
    // }

    char **new;
    new = realloc(files->items, (files->len + 1) * sizeof(char *));
    if (new == NULL) return;
    
    new[files->len] = strdup(s);

    if (new[files->len] == NULL) {
        free(new);
        return;
    }

    files->items = new;
    files->len++;
}

int read_entire_dir(const char *dir_path, Files *files)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return -1;
    }
    
    errno = 0;
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
        FilesAppend(files, ent->d_name);
        ent = readdir(dir);
    }
    closedir(dir);

    if (errno != 0) {
        return -1;
    }

    return 0;
}

int main(void) 
{
    Files files = {0};
    int err = read_entire_dir(".", &files);
    if (err != 0) {
        fprintf(stderr, "ERROR: failed to read directory.\n");
        return 1;
    }

    for (int i = 0; i < files.len; i++) {
        printf("%d: %s\n", i, files.items[i]);
    }
    return 0;
}

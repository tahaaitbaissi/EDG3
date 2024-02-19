#include "fileio.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#include "editor.h"
#include "terminal.h"
#include "syntax.h"
#include "row_op.h"
#include "input.h"
#include "output.h"

#define _BSD_SOURCE

void editorOpen(char *filename)
{
    struct editorConfig *E = GetEditor();
    free(E->filename);
    E->filename = strdup(filename);
    E->numrows = 0;

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editorInsertRow(E->numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E->dirty = 0;
}

char *editorRowsToString(int *buflen)
{
    struct editorConfig *E = GetEditor();
    int totlen = 0;
    int j;
    for (j = 0; j < E->numrows; j++)
        totlen += E->row[j].size + 1;
    *buflen = totlen;

    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < E->numrows; j++) {
        memcpy(p, E->row[j].chars, E->row[j].size);
        p += E->row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorSave(void)
{
    struct editorConfig *E = GetEditor();
    if (E->filename == NULL) {
        E->filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
        if (E->filename == NULL) {
            editorSetStatusMessage("Save aborted");
            return;
        }
        editorSelectSyntaxHighlight();
    }

    int len;
    char *buf = editorRowsToString(&len);
    
    int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                E->dirty = 0; 
                editorSetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

void FilesAppend(Files *files, const char *s)
{
    // if (files->items == NULL) {
    //     new = malloc((files->len + 1) * sizeof(char *));
    // } else {
    //     new = realloc(files->items, (files->len + 1) * sizeof(char *));
    // }

    File *new;
    new = realloc(files->items, (files->len + 1) * sizeof(File));
    if (new == NULL) return;
    
    new[files->len].name = strdup(s);
    new[files->len].len = strlen(s);

    if (new[files->len].name == NULL) {
        free(new);
        return;
    }

    files->items = new;
    files->len++;
}

int file_cmp (const void *a, const void *b) {
    const File *aa = (const File *) a;
    const File *bb = (const File *) b;
    return strcmp(aa->name, bb->name);
}

int read_entire_dir(const char *dir_path, Files *files)
{
    int num_dirs = 0;
    files->items = NULL;
    files->len = 0;
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return -1;
    }
    
    errno = 0;
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
        FilesAppend(files, ent->d_name);
        if (ent->d_type == DT_DIR) {
            files->items[files->len - 1].isdir = 1;
            num_dirs++;
        } else {
            files->items[files->len - 1].isdir = 0;
        }
        ent = readdir(dir);
    }

    closedir(dir);

    File dirs[num_dirs];
    File rst[files->len - num_dirs];
    int j = 0, k = 0;
    for (int i = 0; i < files->len; i++) {
        if (files->items[i].isdir)
        {
            dirs[j] = files->items[i];
            j++;
        } else {
            rst[k] = files->items[i];
            k++;
        }
    }

    qsort(dirs, num_dirs, sizeof(*dirs), file_cmp);
    qsort(rst, files->len - num_dirs, sizeof(*rst), file_cmp);

    k = 0;
    j = 0;

    for (int i = 0; i < files->len; i++) {
        if (j < num_dirs) {
            files->items[i] = dirs[j];
            j++; 
        } else {
            files->items[i] = rst[k];
            k++;            
        }
    }

    if (errno != 0) {
        return -1;
    }

    return 0;
}
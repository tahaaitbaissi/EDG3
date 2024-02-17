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
                E->files.items = NULL;
                E->files.len = 0;
                int err = read_entire_dir(".", &E->files);
                if (err != 0) die("read_entire_dir");
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
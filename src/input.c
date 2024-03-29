#include "input.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "editor.h"
#include "terminal.h"
#include "editor_op.h"
#include "search.h"
#include "output.h"
#include "fileio.h"

void editorProcessKeypress(void)
{
    struct editorConfig *E = GetEditor();
    static int quit_times = ED_QUIT_TIMES;
    int err, len, len_sub;

    int c = editorReadKey();
    switch (c) {
        case '\r':
            if (E->file_browser) {
                if (E->files.items[E->fy].isdir) {
                    len = strlen(E->files.items[E->fy].name);
                    if (E->subdirs == NULL) {
                        E->subdirs = malloc(len + 2);
                        memcpy(E->subdirs, E->files.items[E->fy].name, len);
                        memcpy(E->subdirs + len, "/", 1);
                        memcpy(E->subdirs + len + 1, "\0", 1);
                    } else {
                        len_sub = strlen(E->subdirs);
                        E->subdirs = realloc(E->subdirs, len_sub + len + 1);
                        memcpy(E->subdirs + len_sub, E->files.items[E->fy].name, len);
                        memcpy(E->subdirs + len_sub + len , "/", 1);
                        memcpy(E->subdirs + len_sub + len + 1, "\0", 1);
                    }
                    err = read_entire_dir(E->subdirs, &E->files);
                    if (err != 0) die("read_entire_dir");
                    E->fy = 0;
                } else {
                    editorOpen(E->files.items[E->fy].name);
                }
            } else {
                editorInsertNewLine();
            }
            break;
        case CTRL_KEY('q'):
            if (E->dirty && quit_times > 0) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[?25h", 6);
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
     
        case CTRL_KEY('s'):
            editorSave();
            break;

        case HOME_KEY:
            E->cx = 0;
            break;
        case END_KEY:
            if (E->cy < E->numrows)
                E->cx = E->row[E->cy].size; 
            break;

        case CTRL_KEY('n'):
   			E->fy = 0;         
			editorFileBrowser();
            break;

        case CTRL_KEY('f'):
            editorFind();
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        case PAGE_DOWN:
        case PAGE_UP:
            {
                if (c == PAGE_UP) {
                    E->cy = E->rowoff;
                } else if (c == PAGE_DOWN) {
                    E->cy = E->rowoff + E->screenrows - 1;
                    if (E->cy > E->numrows) E->cy = E->numrows;
                }

                int times = E->screenrows;
                while (times--)
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;

        case ARROW_UP:
        case ARROW_RIGHT:
        case ARROW_LEFT:
        case ARROW_DOWN:
            if (E->file_browser)
            {
                // TODO : select files
                editorSelectfile(c);
            } else {
                editorMoveCursor(c);
            }
            break;
        
        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            editorInsertChar(c);
            break;
    }
    quit_times = ED_QUIT_TIMES;
}

void editorMoveCursor(int key)
{
    struct editorConfig *E = GetEditor();
    erow *row = (E->cy >= E->numrows) ? NULL : E->row + E->cy;

    switch (key) {
        case ARROW_UP:
            if (E->cy != 0) E->cy--;
            break;
        case ARROW_LEFT:
            if (E->cx != 0) {
                E->cx--;
            } else if (E->cy > 0) {
                E->cy--;
                E->cx = E->row[E->cy].size;
            }
            break;
        case ARROW_DOWN:
            if (E->cy < E->numrows) E->cy++;
            break;
        case ARROW_RIGHT:
            if (row && E->cx < row->size) {
                E->cx++;
            } else if (row && E->cx == row->size) {
                E->cy++;
                E->cx = 0;
            }
            break;
    }

    row = (E->cy >= E->numrows) ? NULL : E->row + E->cy;
    int rowlen = row ? row->size : 0;
    if (E->cx > rowlen) {
        E->cx = rowlen;
    }
}

char *editorPrompt(char *prompt, void (*callback)(char *, int))
{
    size_t bufsize = 128;
    char *buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        editorSetStatusMessage(prompt, buf);
        editorRefreshScreen();

        int c = editorReadKey();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editorSetStatusMessage("");
            if (callback) callback(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                editorSetStatusMessage("");
                if (callback) callback(buf, c);
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback) callback(buf, c);
    }
}

void editorSelectfile(int key) 
{
    struct editorConfig *E = GetEditor();

    switch (key) {
        case ARROW_UP:
            if (E->fy != 0) E->fy--;
            break;

        case ARROW_DOWN:
            if (E->fy < E->files.len) E->fy++;
            break;
    }
}

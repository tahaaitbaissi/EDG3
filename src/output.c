#include "output.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "editor.h"
#include "terminal.h"
#include "fileio.h"
#include "defines.h"
#include "syntax.h"
#include "row_op.h"

void editorRefreshScreen(void)
{
    struct editorConfig *E = GetEditor();
    if (!E->file_browser) {
        editorScroll();
    } else {
        editorFileBrowserScroll();
    }

    struct abuf ab = ABUF_INIT;
    
    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);
    
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E->cy - E->rowoff) + 1, E->rx - E->coloff + 1);
    abAppend(&ab, buf, strlen(buf));

    // abAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorDrawStatusBar(struct abuf *ab)
{
    struct editorConfig *E = GetEditor();
    abAppend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", 
            E->filename ? E->filename : "[No Name]", 
            E->numrows, E->dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d", 
            E->syntax ? E->syntax->filetype : "no ft", E->cy + 1, E->numrows);
    if (len > E->screencols) len = E->screencols;
    abAppend(ab, status, len);
    while (len < E->screencols) {
        if (E->screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }

    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorDrawRows(struct abuf *ab)
{
    struct editorConfig *E = GetEditor();
    int y;
    for (y = 0; y < E->screenrows; y++) {
        int filerow = y + E->rowoff;
        if (E->file_browser) {
            // TODO: Scrolling in filebrowser
            if (filerow >= E->files.len) {
                abAppend(ab, "\x1b[K", 3);
                abAppend(ab, "\r\n", 2);
                continue;
            }
            char buf[30];
            int buflen = snprintf(buf, sizeof(buf), "%s", E->files.items[filerow].name);
            abAppend(ab, "\x1b[?25l", 6);
            if (filerow == E->fy)
                abAppend(ab, "\x1b[7m", 4);
            if (E->files.items[filerow].isdir)
                abAppend(ab, "\x1b[36m", 5);
            abAppend(ab, buf, buflen);
            abAppend(ab, "\x1b[m", 3);
        } else {
            if (filerow >= E->numrows) {
                if (E->numrows == 0 && y == E->screenrows / 3) {
                    char welcome[80];
                    int welcomeLen = snprintf(welcome, sizeof(welcome), "EDG3");
                    if (welcomeLen > E->screencols) welcomeLen = E->screencols;
                    int padding = (E->screencols - welcomeLen) / 2;
                    if (padding) {
                        abAppend(ab, "~" , 1);
                        padding--;
                    }
                    while (padding--) abAppend(ab, " ", 1);
                    abAppend(ab, welcome, welcomeLen);
                } else { 
                    abAppend(ab, "~", 1);
                }
            } else {
                abAppend(ab, "\x1b[?25h", 6);
                int len = E->row[filerow].rsize - E->coloff;
                if (len < 0) len = 0;
                if (len > E->screencols) len = E->screencols;
                char *c = E->row[filerow].render + E->coloff;
                unsigned char *hl = E->row[filerow].hl + E->coloff;
                int current_color = -1;
                int j;
                for (j = 0; j < len; j++) {
                    if (iscntrl(c[j])) {
                        char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                        abAppend(ab, "\x1b[7m", 4);
                        abAppend(ab, &sym, 1);
                        abAppend(ab, "\x1b[m", 3);
                        if (current_color != -1) {
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm",
                                                current_color);
                            abAppend(ab, buf, clen);
                        }
                    } else if (hl[j] == HL_NORMAL) {
                        if (current_color != -1) {
                            abAppend(ab, "\x1b[39m", 5);
                            current_color = -1;
                        }
                        abAppend(ab, c + j, 1);
                    } else {
                        int color = editorSyntaxToColor(hl[j]);
                        if (color != current_color) {
                            current_color = color;
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
                            abAppend(ab, buf, clen);
                        }
                        abAppend(ab, c + j, 1);
                    }
                }
                abAppend(ab, "\x1b[39m", 5);
                //abAppend(ab, E->row[filerow].render + E->coloff, len);
            }
        }
        
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

void editorSetStatusMessage(const char *fmt, ...)
{
    struct editorConfig *E = GetEditor();
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
    va_end(ap);
    E->statusmsg_time = time(NULL);
}

void editorDrawMessageBar(struct abuf *ab)
{
    struct editorConfig *E = GetEditor();
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(E->statusmsg);
    if (msglen > E->screencols) msglen = E->screencols;
    if (msglen && time(NULL) - E->statusmsg_time < 5)
        abAppend(ab, E->statusmsg, msglen);
}

void editorScroll(void)
{
    struct editorConfig *E = GetEditor();
    E->rx = 0;
    if (E->cy < E->numrows) {
        E->rx = editorRowCxToRx(E->row + E->cy, E->cx);
    }

    if (E->cy < E->rowoff) {
        E->rowoff = E->cy;
    }
    if (E->cy >= E->rowoff + E->screenrows) {
        E->rowoff = E->cy - E->screenrows + 1;
    }
    if (E->rx < E->coloff) {
        E->coloff = E->rx;
    }
    if (E->rx >= E->coloff + E->screencols) {
        E->coloff = E->rx - E->screencols + 1;
    }
}

void editorFileBrowserScroll(void)
{
    struct editorConfig *E = GetEditor();
    
    if (E->fy < E->rowoff) {
        E->rowoff = E->fy;
    }
    if (E->fy >= E->rowoff + E->screenrows) {
        E->rowoff = E->fy - E->screenrows + 1;
    }
    
}
void editorFileBrowser(void)
{
    int err;
    struct editorConfig *E = GetEditor();
    E->subdirs = NULL;
    err = read_entire_dir(".", &E->files);
    if (err != 0) die("read_entire_dir");
    E->file_browser = !E->file_browser;
}
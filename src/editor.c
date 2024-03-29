#include "editor.h"
#include "terminal.h"
#include "fileio.h"

struct editorConfig E;

struct editorConfig *GetEditor(void)
{
    return &E;
}

void initEditor(void)
{
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.fy = 0;
    E.numrows = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.row = NULL;
    E.filename = NULL;
    E.subdirs = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.dirty = 0;
    E.syntax = NULL;
    E.file_browser = 0;
    E.files.items = NULL;
    E.files.len = 0;


    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}
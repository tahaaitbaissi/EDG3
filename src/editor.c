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
    int err;
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.fy = 0;
    E.numrows = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.row = NULL;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.dirty = 0;
    E.syntax = NULL;
    E.file_browser = 0;
    err = read_entire_dir(".", &E.files);
    if (err != 0) die("read_entire_dir");


    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}
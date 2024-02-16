#include <stdio.h>

#include "editor.h"
#include "terminal.h"
#include "output.h"
#include "input.h"
#include "fileio.h"


int main(int argc, char **argv)
{
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    editorSetStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-F = find |Ctrl-Q = quit");

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress(); 
    }
    return 0;
}
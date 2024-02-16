#include "editor_op.h"

#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "row_op.h"


void editorInsertChar(int c)
{
    struct editorConfig *E = GetEditor();
    if (E->cy == E->numrows) {
        editorInsertRow(E->numrows, "", 0);
    }
    editorRowInsertChar(E->row + E->cy, E->cx, c);
    E->cx++;
}

void editorDelChar(void)
{
    struct editorConfig *E = GetEditor();
    if (E->cy == E->numrows) return;
    if (E->cx == 0 && E->cy == 0) return;

    erow *row = E->row + E->cy;
    if (E->cx > 0) {
        editorRowDelChar(row, E->cx - 1);
        E->cx--;
    } else {
        E->cx = E->row[E->cy - 1].size;
        editorRowAppendString(E->row + E->cy - 1, row->chars, row->size);
        editorDelRow(E->cy);
        E->cy--;
    }
}

void editorFreeRow(erow *row)
{
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editorDelRow(int at)
{
    struct editorConfig *E = GetEditor();
    if (at < 0 || at >= E->numrows) return;
    editorFreeRow(E->row + at);
    memmove(E->row + at, E->row + at + 1, sizeof(erow) * (E->numrows - at - 1));
    for (int j = at; j < E->numrows - 1; j++) E->row[j].idx--;
    E->numrows--;
    E->dirty++;
}

void editorInsertNewLine(void)
{
    struct editorConfig *E = GetEditor();
    if (E->cx == 0) {
        editorInsertRow(E->cy, "", 0);
    } else {
        erow *row = E->row + E->cy;
        editorInsertRow(E->cy + 1, row->chars + E->cx, row->size - E->cx);
        row = E->row + E->cy;
        row->size = E->cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E->cy++;
    E->cx = 0;
}
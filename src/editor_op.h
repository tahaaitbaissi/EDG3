#ifndef EDITOR_OP_H_
#define EDITOR_OP_H_

#include "defines.h"

void editorInsertChar(int c);
void editorDelChar(void);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorInsertNewLine(void);

#endif
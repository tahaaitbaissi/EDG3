#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <stdlib.h>
#include "defines.h"

void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
int is_separator(int c);
void editorSelectSyntaxHighlight(void);

#endif

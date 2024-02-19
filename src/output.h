#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <stdarg.h>
#include "abuf.h"

void editorRefreshScreen(void);
void editorDrawRows(struct abuf *ab);
void editorScroll(void);
void editorDrawStatusBar(struct abuf *ab);
void editorSetStatusMessage(const char *fmt, ...);
void editorDrawMessageBar(struct abuf *ab);
void editorFileBrowser(void);

#endif
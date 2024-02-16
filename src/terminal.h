#ifndef TERMINAL_H_
#define TERMINAL_H_

void enableRawMode(void);
void disableRawMode(void);
void die(const char *s);
int editorReadKey(void);
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);

#endif
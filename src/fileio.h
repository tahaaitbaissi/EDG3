#ifndef FILE_IO_H_
#define FILE_IO_H_

void editorOpen(char *filename);
char *editorRowsToString(int *buflen);
void editorSave(void);

#endif
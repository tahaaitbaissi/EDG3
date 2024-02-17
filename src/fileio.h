#ifndef FILE_IO_H_
#define FILE_IO_H_

#include "defines.h"

void editorOpen(char *filename);
char *editorRowsToString(int *buflen);
void editorSave(void);
int read_entire_dir(const char *dir_path, Files *files);
void filesAppend(Files *files, const char *s);

#endif
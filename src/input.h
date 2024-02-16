#ifndef INPUT_H_
#define INPUT_H_

void editorProcessKeypress(void);
void editorMoveCursor(int key);
char *editorPrompt(char *prompt, void (*callback)(char *, int));

#endif
main: src/main.c src/abuf.c src/editor_op.c src/editor.c src/fileio.c src/input.c src/output.c src/row_op.c src/search.c src/syntax.c src/terminal.c
	$(CC) src/*.c -o main -Wall -Wextra -pedantic

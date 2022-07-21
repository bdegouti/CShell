all: cshell.c cshell.h
	gcc -g -Wall -o cshell_processes cshell.c -lm
clean:
	rm cshell
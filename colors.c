/*
The MIT License (MIT)

Copyright (c) 2014 Karol Farbiś

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define STRING "Quick brown fox jumps over the lazy dog"

static struct termios original;

int max(int a, int b) {
	if(a > b) return a;
	return b;
}

void initialize_terminal(void) {
	struct termios term;
	tcgetattr(STDIN_FILENO, &term);
	original = term;

	term.c_lflag &= ~ECHO;
	term.c_lflag &= ~ICANON;

	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void finalize_terminal(void) {
	tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

static char* buffer;
static size_t buffer_size;
static size_t buffer_length;

void clear_buffer() {
	free(buffer);
	buffer = NULL;
	buffer_size = buffer_length = 0;
}

void resize_buffer(size_t new_size) {
	buffer = realloc(buffer, new_size * sizeof(char));
	if(new_size == 0) buffer = NULL;
	buffer_size = new_size;
}

void delete_last_character() {
	if(buffer_length == 0) return;
	buffer[--buffer_length] = 0;
	if(buffer_length * 4 < buffer_size) {
		resize_buffer(buffer_size / 4);
	}
}

void add_character(char c) {
	if(buffer_length + 1 > buffer_size) {
		resize_buffer(max(6, buffer_size) * 2);
	}
	buffer[buffer_length++] = c;
	buffer[buffer_length] = 0;
}

void write_back() {
	printf("\r\033[K\n\033[K\033[A");

	if(buffer != NULL) {
		char* copy_buffer = malloc(buffer_length * sizeof(char) + 1);
		strcpy(copy_buffer, buffer);
		if(buffer_length > 0 && copy_buffer[buffer_length-1] == ';')
			copy_buffer[buffer_length-1] = 0;
		printf("\033[%sm%s\n", copy_buffer, buffer);
		free(copy_buffer);
	} else
		printf("\n");
	printf("%s\r\033[0m\033[A\033[%zuC\033[D", STRING, buffer_length+1);
}

bool read_and_parse_character() {
	char c;
	if(scanf("%c", &c) == EOF) return false;

	switch(c) {
		case 4: 
			return false;

		case 10:
			clear_buffer();
			break;

		case 127:
			delete_last_character();
			break;

		default:
			add_character(c);
	}

	write_back();

	return true;
}

int main(void) {
	initialize_terminal();
	printf("\n%s\r\033[A", STRING);

	while(read_and_parse_character());

	free(buffer);

	finalize_terminal();

	return 0;
}

#ifndef TINY_CONSOLE_H
#define TINY_CONSOLE_H

#include <stddef.h>

void console_init(void);
void console_putc(char ch);
void console_puts(const char *text);
int console_poll_line(char *line, size_t capacity);

#endif

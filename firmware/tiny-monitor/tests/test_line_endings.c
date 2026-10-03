#ifdef HOST_TEST
#include <assert.h>
#include <stdio.h>
static unsigned int count_lines(const char *s)
{
    int cr = 0; unsigned int lines = 0;
    while (*s) { if (*s == '\n' && cr) cr = 0; else if (*s == '\r' || *s == '\n') { ++lines; cr = *s == '\r'; } else cr = 0; ++s; }
    return lines;
}
int main(void)
{
    assert(count_lines("a\r") == 1); assert(count_lines("a\n") == 1);
    assert(count_lines("a\r\n") == 1); assert(count_lines("a\r\nb\r\n") == 2);
    puts("line ending tests: PASS"); return 0;
}
#endif

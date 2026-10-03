#include "parser.h"
#include <ctype.h>

int parser_parse_u32(const char *text, uint32_t *value)
{
    uint32_t result = 0; unsigned int base = 10, digit; const char *p = text;
    if (p == 0 || *p == '\0' || *p == '-') return 0;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) { base = 16; p += 2; }
    if (*p == '\0') return 0;
    while (*p != '\0') {
        if (*p >= '0' && *p <= '9') digit = (unsigned int)(*p - '0');
        else if (*p >= 'a' && *p <= 'f') digit = (unsigned int)(*p - 'a') + 10U;
        else if (*p >= 'A' && *p <= 'F') digit = (unsigned int)(*p - 'A') + 10U;
        else return 0;
        if (digit >= base || result > (UINT32_MAX - digit) / base) return 0;
        result = result * base + digit; ++p;
    }
    *value = result; return 1;
}

int parser_range_valid(uint32_t addr, uint32_t len)
{
    return addr < UINT32_C(0x10000) && len <= UINT32_C(0x10000) - addr;
}

size_t parser_tokenize(char *line, char **argv, size_t max_args)
{
    size_t argc = 0;
    while (*line != '\0') {
        while (isspace((unsigned char)*line) || *line == ',' || *line == '(' || *line == ')') ++line;
        if (*line == '\0') break;
        if (argc == max_args) return max_args + 1U;
        argv[argc++] = line;
        while (*line != '\0' && !isspace((unsigned char)*line) && *line != ',' && *line != '(' && *line != ')') ++line;
        if (*line != '\0') *line++ = '\0';
    }
    return argc;
}

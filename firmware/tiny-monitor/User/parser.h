#ifndef TINY_PARSER_H
#define TINY_PARSER_H
#include <stddef.h>
#include <stdint.h>
int parser_parse_u32(const char *text, uint32_t *value);
int parser_range_valid(uint32_t addr, uint32_t len);
size_t parser_tokenize(char *line, char **argv, size_t max_args);
#endif

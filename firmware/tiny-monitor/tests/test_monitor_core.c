#ifdef HOST_TEST
#include "parser.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    uint32_t v; char line[] = "  save 0 100 f "; char *argv[4];
    assert(parser_parse_u32("0", &v) && v == 0);
    assert(parser_parse_u32("255", &v) && v == 255);
    assert(!parser_parse_u32("ff", &v));
    assert(parser_parse_u32("0x10000", &v) && v == 0x10000);
    assert(!parser_parse_u32("", &v) && !parser_parse_u32("-1", &v));
    assert(!parser_parse_u32("100000000", &v) && !parser_parse_u32("xyz", &v));
    assert(parser_range_valid(0, 0x10000) && parser_range_valid(0xffff, 1));
    assert(!parser_range_valid(0xffff, 2) && !parser_range_valid(0x10000, 0));
    assert(!parser_range_valid(1, UINT32_MAX));
    assert(parser_tokenize(line, argv, 4) == 4);
    assert(!strcmp(argv[0], "save") && !strcmp(argv[3], "f"));
    puts("monitor_core tests: PASS"); return 0;
}
#endif

#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct parser{
    uint32_t partition;
    uint32_t sector;
    uint32_t verbose;
    char *imagefile;
    char *srcpath; /*used for path in minls*/
    char *dstpath;
} parser;

void print_usage();
int parse_line(struct parser *parse, int argc, char **argv);



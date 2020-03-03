#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>

typedef struct parser{
    uint_32 partition;
    uint_32 sector;
    uint_32 verbose;
    char *imagefile;
    char *srcpath;
    char *dstpath;
} parser;

void print_usage();
void parse_line(struct parser);



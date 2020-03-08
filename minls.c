#include "min_funcs.h"

int main(int argc, char **argv)
{
    parser p;
    finder f;
    part_table t;
    superblock s;
    int32_t check;
    int file = 0;
    //int testint;
    init_parser(&p);
    init_finder(&f);

    if((check = parse_line_ls(&p, argc, argv))){
        exit(1);
    }

    if(check = find_filesystem(&p, &f, &t)){
        return 1;
    }

    check_SB(&f, &s);

    if(p.verbose){
        verbose2(&p, &f, &t, &s);
    }

    close(file);
    printf("test print since nothing else is doing anything\n");
    return 0;
}

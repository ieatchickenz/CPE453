#include "min_funcs.h"

int main(int argc, char **argv){
    parser p;
    finder f;
    part_table t;
    uint32_t check;
    int file = 0;
    int testint;
    init_parser(&p);
    init_finder(&f);

    if((check = parse_line(&p, argc, argv))){
        exit(1);
    }

    find_filesystem(&p, &f, &t);
    
    if(p.verbose){
        verbose2(&p, &f, &t);
    }

    close(file);
    return 0;
}


#include "min_funcs.h"


void init_parser(parser *p){
   p->partition = 0;
   p->sector    = 0;
   p->verbose   = 0;
}

int parse_line_ls(struct parser *parse, int argc, char **argv){
    int32_t c;
    char *endptr;
    extern char *optarg;
    extern int optind;

    while((c=getopt(argc, argv, "hvp:s:")) != -1){
        switch(c){
            case 'v':
                parse->verbose += 1;
                break;
            case 'p':
                if(parse->partition){ /*can't have more than one*/
                    print_usage_ls();
                    return 1;
                }
                parse->partition = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage_ls();
                    return 1;
                }
                break;

            case 's':
                if(parse->sector){  /*can't have more than one*/
                    print_usage_ls();
                    return 1;
                }
                parse->sector = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage_ls();
                    return 1;
                }
                break;

            case 'h':
                print_usage_ls();
                return 1;
                break;

            default:
                break;
        }
    }


    if(!(parse->imagefile = argv[optind++])){
        print_usage_ls();
        return 1;
    }

    if(argc >= optind){
       parse->srcpath = argv[optind++];
    }
    parse->dstpath = NULL;
    /* this part is exclusively for minget
    if(argc >= optind){
        parse->dstpath = argv[optind];
    }*/

    if(argc > optind || argc < 2){
        print_usage_ls();
        return 1;
    }

    return 0;
}

void print_usage_ls(){
    printf("usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
    printf("Options:\n\t-p  part    --- select partition for filesystem");
    printf("(default");
    printf(": none)\n\t-s  sub     --- select subpartition for filesystem");
    printf("(default: none)\n\t-h  help    --- print usage information and");
    printf(" exit\n\t-v  verbose --- increase verbosity level\n");
}

void print_usage_get(){
    printf("usage: minls [-v] [-p part [-s subpart]] imagefile srcpath");
    printf(" [dstpath]\n");
    printf("Options:\n\t-p  part    --- select partition for filesystem");
    printf("(default");
    printf(": none)\n\t-s  sub     --- select subpartition for filesystem");
    printf("(default: none)\n\t-h  help    --- print usage information and");
    printf(" exit\n\t-v  verbose --- increase verbosity level\n");
}

int parse_line_get(struct parser *parse, int argc, char **argv){
    int32_t c;
    char *endptr;
    extern char *optarg;
    extern int optind;

    while((c=getopt(argc, argv, "vp:s:")) != -1){
        switch(c){
            case 'v':
                parse->verbose += 1;
                break;
            case 'p':
                if(parse->partition){   /*can't have more than one*/
                    print_usage_get();
                    return 1;
                }
                parse->partition = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage_get();
                    return 1;
                }
                break;

            case 's':      /*can't have more than one*/
                if(parse->sector){
                    print_usage_get();
                    return 1;
                }
                parse->sector = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage_get();
                    return 1;
                }
                break;

            default:
                break;
        }
    }


    if(!(parse->imagefile = argv[optind++])){
        print_usage_get();
        return 1;
    }

    if(argc >= optind){
       if(!(parse->srcpath = argv[optind++])){
           print_usage_get();
           return 1;
       }
    }

    if(argc >= optind){
        parse->dstpath = argv[optind++];
    }
    else{
        parse->dstpath = NULL;
    }

    if(argc > optind || argc < 3){ /*3 bc srcfile here*/
        print_usage_get();
        return 1;
    }

    return 0;
}

int openfile(struct parser *p, int *file){
   *file = open( (p->imagefile), O_RDONLY);
   return *file;
}

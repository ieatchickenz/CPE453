#include "min_funcs.h"
int parse_line(struct parser *parse, int argc, char **argv);
void print_usage();

int main(int argc, char **argv){
    struct parser p;
    uint32_t check;

    if((check=parse_line(&p, argc, argv))){
        exit(1);
    }

    printf("args:%d\nv count: %d\np: %d\ns: %d\n", argc,p.verbose, p.partition,
            p.sector);
    printf("imagefile: %s\n", p.imagefile);
    printf("srcfile: %s\n", p.srcpath);
    if(p.dstpath){
        printf("dstpath: %s\n", p.dstpath);
    }
    return 0;
}

int parse_line(struct parser *parse, int argc, char **argv){
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
                    print_usage();
                    return 1;
                }
                parse->partition = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage();
                    return 1;
                }
                break;

            case 's':      /*can't have more than one*/
                if(parse->sector){
                    print_usage();
                    return 1;
                }
                parse->sector = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage();
                    return 1;
                }
                break;

            default:
                break;
        }
    }


    if(!(parse->imagefile = argv[optind++])){
        print_usage();
        return 1;
    }

    if(argc >= optind){
       parse->srcpath = argv[optind++];
    }

    if(argc >= optind){
        parse->dstpath = argv[optind++];
    }

    if(argc > optind || argc < 3){ /*3 bc srcfile here*/
        print_usage();
        return 1;
    }

    return 0;
}


void print_usage(){
    printf("usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
}

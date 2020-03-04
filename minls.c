#include "min.h" 

void main(int argc, char **argv){
    struct parser p;
    uint32_t check;
    
    if((check=parse_line(&p, argc, argv))){
        exit(1);
    }

    printf("v count: %d\np: %d\ns: %d\n", p.verbose, p.partition, 
            p.sector);
    printf("imagefile: %s\n", p.imagefile);
    if(p.srcpath){
        printf("optional path: %s\n", p.srcpath);
    }   

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
                parse->partition = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage();
                    return 1;
                }
                break;

            case 's':
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

    /*this will be different for minget 7 instead of 6*/
    if(argc > 7+(parse->verbose) || argc < 2){
        print_usage();
        return 1;
    }

    if(!(parse->imagefile = argv[optind++])){
        print_usage();
        return 1;
    }

    if(argc >= optind){
       parse->srcpath = argv[optind++]; 
    }
    if(argc >= optind){
        parse->dstpath = argv[optind];
    }

    return 0;
}


void print_usage(){
    printf("usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
}

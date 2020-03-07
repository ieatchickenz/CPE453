#include "min_funcs.h"

void init_parser(parser *p){
   p->partition = 0;
   p->sector    = 0;
   p->verbose   = 0;
}
/* off_t lseek(int fd, off_t offset, int whence); */
/* ssize_t read(int fd, void *buf, size_t count); */
/* valid partition table contains a signature:
   0x55 in byte 510, and 0xAA in byte 511 */
void init_parser(parser *p);
uint32_t check_part(parser *p, int file, part_table *part){
   intptr_t offset, test;
   char first, second;
   if(-1 == lseek(file, SIG1LOC, SEEK_SET)){
      perror("lseek");
      return 0;
   }

   if(-1 == read(file, &first, sizeof(char))){
      perror("lseek");
      return 0;
   }
   if(-1 == read(file, &second, sizeof(char))){
      perror("lseek");
      return 0;
   }
   assert(fprintf(stderr, "%d and & %d this code is not working I think\n",first, second ));
   // part->entry[p->partition].last_head = 'B'; /* for testing*/
   //
   // offset = (intptr_t)(&(part->entry[p->partition]));
   // test = offset + 510;
   // if ( (int)(*((&(part->entry[p->partition]))+(510))) != 0x55 ||
   //     (part->entry[p->partition])[511] != 0xAA) {
   //    /* Invalid partition table. */
   //    return 0;
   // }

   /*assert(fprintf(stderr, "%s\n", ));*/


   return 0;
}
int check_SB(){
   return 0;
}
int check_DIR(){
   return 0;
}
int check_file(){
   return 0;
}
int LBA_convert(){
   return 0;
}
int logzonesize(){
   return 0;
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

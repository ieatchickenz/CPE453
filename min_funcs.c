#include "min_funcs.h"

void init_parser(parser *p){
   p->partition = -1;
   p->sector    = -1;
   p->verbose   = 0;
   p->imagefile = NULL;
   p->srcpath = NULL;
   p->dstpath = NULL;
}

void init_finder(finder *f){
    f->offset = 0;
    f->fd = 0;
}

uint32_t check_part(int32_t which, finder *f, part_table *part){
   /*returns 0 on success and 1 on failure*/

   uint8_t check_bytes[2];
   off_t offset = f->offset;
   uint32_t file = f->fd;

   /*checking validity of partition table first*/
   if(-1 == lseek(file, offset, SEEK_SET)){  /*first seek to the boot block you're interested in*/
       perror("lseek");
       return 1;
   }

   if(-1 == lseek(file, SIG1LOC, SEEK_CUR)){  /*next seek to byte 510*/
       perror("lseek");
       return 1;
   }
   /*next fill an array with the bytes we're interested in and check*/
   if(-1 == read(file, check_bytes, 2)){
      perror("read");
      return 1;
   }

   /*do the actual check*/
   if(check_bytes[0] != SIG1 || check_bytes[1] != SIG2){
       fprintf(stderr, "This is not a valid partition table");
       return 1;
   }
   assert(fprintf(stderr, "partition signatures are %02X and %02X\n",
                           check_bytes[0] , check_bytes[1]));
   /*after checking for validity, fill the part_table*/
   if(-1 == lseek(file, offset, SEEK_SET)){  /*relocate the boot block we want*/
      perror("lseek");
      return 1;
   }

   if(-1 == lseek(file, PARTS, SEEK_CUR)){ /*now find the start of the tables*/
      perror("lseek");
      return 1;
   }

   if(-1 == read(file, part, sizeof(struct part_table))){ /*fill the part_table struct*/
      perror("read");
      return 1;
   }

   /*here we need to find the new offset*/
   if((offset = find_offset(which, part)) == -1){
       return 1;
   }
   f->offset = offset;
   /*set the new offset so we can now use this for partition and subpartition*/


   return 0;
}

off_t find_offset(int32_t which, struct part_table *part){
    off_t offset;
    /*check if it's a MINIX style partition*/
    if(part->entry[which].sysind != MINIX){
        fprintf(stderr, "This is not a valid MINIX partition.\n");
        return -1;
    }

    offset = (part->entry[which].lowsec) * 512;
    return offset;
}

uint32_t find_filesystem(parser *p, finder *f, part_table *part){
    /* returns 0 on success and 1 on failure*/
    int32_t check = 0;

    if( -1 == (check = openfile(p, f)) ){
      return 1;
    }

    if(p->partition > 0 && p->partition < 4){
        if( (check = check_part(p->partition, f, part)) ){
            return 1;
        }

        if(p->sector > 0 && p->sector < 4){
            if( (check = check_part(p->sector, f, part)) ){
                return 1;
            }
        }
        else{
            if(p->sector != -1){
                fprintf(stderr, "Valid partition is only 0-3\n");
                return 1;
            }
        }
    }
    else{
        if(p->partition != -1){
          fprintf(stderr, "Valid partition is only 0-3\n");
          return 1;
        }
        /*otherwise just read starting from the beginging*/
    }
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

    if(parse->sector != -1 && parse->partition == -1){
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

    if(parse->sector != -1 && parse->partition == -1){ /*can not have subpartition with no partition*/
        print_usage_get();
        return 1;
    }
    return 0;
}

int32_t openfile(struct parser *p, struct finder *f){
   if( -1 == (f->fd = open((p->imagefile), O_RDONLY))){
      perror("open");
      return f->fd;
   } /*open returns an int*/
   return 0; /*****************************************************CHECK THIS*********************not shure*/
}

void verbose1(){

}

void verbose2(parser *p, finder *f, part_table *part){

    int i;
    verbose1();
    printf("Parser:\n");
    printf("\tPartition: %d\n", p->partition);
    printf("\tSubpartition: %d\n", p->sector);
    printf("\tHow Verbose: %d\n", p->verbose);
    printf("\tImageFile: %s\n", p->imagefile);
    printf("\tSource Path(path for ls): %s\n", p->srcpath);
    printf("\tDest. Path: %s\n", p->dstpath);

    printf("\n");

    printf("Finder:\n");
    printf("\tOffset: %ld\n", f->offset);
    printf("\tFile Descriptor: %d\n", f->fd);

    printf("\n");

    for(i=0;i<4;i++){
        printf("Entry [%d]:\n", i);
        printf("\tBoot Ind%d\n", part->entry[i].bootind);
        printf("\tStart (Head, Sec, Cyl): %d, %d, %d\n",
                part->entry[i].start_head, part->entry[i].start_sec, part->entry[i].start_cyl);
        printf("\tSys Ind%d\n", part->entry[i].sysind);
        printf("\tLast (Head, Sec, Cyl): %d, %d, %d\n",
                part->entry[i].last_head, part->entry[i].last_sec, part->entry[i].last_cyl);
        printf("\tFirt Sector %lu\n", part->entry[i].lowsec);
        printf("\tSize %lu\n", part->entry[i].size);

        printf("\n\n");
    }

}






















/* end */

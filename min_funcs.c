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
/* off_t lseek(int fd, off_t offset, int whence); */
/* ssize_t read(int fd, void *buf, size_t count); */
/* valid partition table contains a signature:
   0x55 in byte 510, and 0xAA in byte 511 */

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
   f->offset = offset; /*set the new offset so we can now use this for partition and subpartition*/

  /*-----------------------------------------this is where I modified up to------------------------------*/

   /*
   part->entry[p->partition].last_head = 'B'; for testing
   offset = (intptr_t)(&(part->entry[p->partition]));
   test = offset + 510;*/
   // if ( (int)(*((&(part->entry[p->partition]))+(510))) != 0x55 ||
   //     (part->entry[p->partition])[511] != 0xAA) {
   //    /* Invalid partition table. */
   //    return 0;
   // }

   /*assert(fprintf(stderr, "%s\n", ));*/


   return 0;
}

off_t find_offset(int32_t which, struct part_table *part){
    off_t offset;
    if(part->entry[which].type != MINIX){ /*check if it's a MINIX style partition*/
        fprintf(stderr, "This is not a valid MINIX partition.\n");
        return -1;
    }

    offset = (part->entry[which].lowsec) * 512;
    return offset;
}

uint32_t find_filesystem(parser *p, finder *f, part_table *part){
    /* returns 0 on success and 1 on failure*/
    uint32_t check = 0;

    if((check = openfile(p, f)) == -1){
      return 1;
    }

    if(p->partition > 0 && p->partition < 4){
        if(check = check_part(p->partition, f, part){
            return 1;
        }

        if(p->sector > 0 && p->sector < 4){
            if(check = check_part(p->sector, f, part){
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
   if((f->fd = open((p->imagefile), O_RDONLY)) == -1){
      perror("open");
      return f->fd;
   } /*open returns an int*/
}

void verbose1(){/*in main put switch statement that decides which verbose to run*/
    /*this verbose is reserved for superblocks and inode*/
}

void verbose2(parser *p, finder *f, part_table *part){
    /*this verbose is reserved for verbose1, and the parsing, finder and part_table structs*/
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
    printf("\tOffset: %d\n", f->offset);
    printf("\tFile Descriptor: %d\n", f->fd);

    printf("\n");

    for(i=0;i<4;i++){
        printf("Entry [%d]:\n", i);
        printf("\tBoot Ind%d\n", part[i].bootind);
        printf("\tStart (Head, Sec, Cyl): %d, %d, %d\n",
                part[i].start_head, part[i].start_sec, part[i].start_cyl);
        printf("\tSys Ind%d\n", part[i].sysind);
        printf("\tLast (Head, Sec, Cyl): %d, %d, %d\n",
                part[i].last_head, part[i].last_sec, part[i].last_cyl);
        printf("\tFirt Sector%d\n", part[i].lowsec);
        printf("\tSize%d\n", part[i].size);

        printf("\n\n");
    }

}

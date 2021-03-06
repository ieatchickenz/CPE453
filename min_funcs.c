#include "min_funcs.h"

void init_parser(parser *p){
   assert(fprintf(stderr, "init_parser()\n"));
   p->partition = -1;
   p->sector    = -1;
   p->verbose   = 0;
   p->imagefile = NULL;
   p->srcpath = NULL;
   p->dstpath = NULL;
   p->current[60] = '\0';
   p->compare[60] = '\0';
}

int32_t openfile(struct parser *p, struct finder *f){
   assert(fprintf(stderr, "openfile()\n"));
   if( -1 == (f->fd = open((p->imagefile), O_RDONLY))){
      perror("open");
      return -1;
   } /*open returns an int*/
   return 0;
   /****************************CHECK THIS*********************not shure*/
}

void init_finder(finder *f){
   assert(fprintf(stderr, "init_finder()\n"));
    f->offset = 0;
    f->fd = 0;
    f->zonesize = 0;
    f->last_sector = 0;
    f->partoff = 0;
}

void init_part_table(part_table *t){
   assert(fprintf(stderr, "init_part_table()\n"));
   memset(t, 0, sizeof(struct part_table));
}

/*returns 0 on success and 1 on failure*/
uint32_t check_part(int32_t which, finder *f, part_table *part){
   assert(fprintf(stderr, "check_part()\n"));
   uint8_t check_bytes[2];
   off_t offset = f->offset;
   uint32_t file = f->fd;

   /*checking validity of partition table first*/
   /*first seek to the boot block you're interested in*/
   if(-1 == lseek(file, offset, SEEK_SET)){
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

   if(-1 == read(file, part, sizeof(struct part_table))){
      /*fill the part_table struct*/
      perror("read");
      return 1;
   }

   /*here we need to find the new offset*/
   if( -1 == (offset = find_offset(which, part)) ){
       return 1;
   }
   f->offset  = offset;
   f->partoff = offset;
   /*set the new offset so we can now use this for partition and subpartition*/


   return 0;
}

off_t find_offset(int32_t which, struct part_table *part){
   assert(fprintf(stderr, "find_offset()\n"));
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
    assert(fprintf(stderr, "find_filesystem()\n"));
    uint32_t which = 0;
    int32_t check = 0;

    if( -1 == (check = openfile(p, f)) ){
      return 1;
    }

    if(p->partition >= 0 && p->partition < 4){
        if( (check = check_part(p->partition, f, part)) ){
            return 1;
        }

        which = p->partition;

        if(p->sector >= 0 && p->sector < 4){
            if( (check = check_part(p->sector, f, part)) ){
                return 1;
            }
            which = p->sector;
        }
        else{
            if(p->sector != -1){
                fprintf(stderr, "Valid partition is only 0-3\n");
                return 1;
            }
        }
        f->last_sector = part->entry[which].lowsec + part->entry[which].size-1;
    }
    else{
        if(p->partition != -1){
          f->last_sector = 0;
          fprintf(stderr, "Valid partition is only 0-3\n");
          return 1;
        }
        /*otherwise just read starting from the beginging*/
    }

    /*find the last sector of the partition*/
    assert(fprintf(stderr, "last: %u\n", f->last_sector));
    return 0;
}

int check_SB(finder *f, superblock *s){
    /*returns 0 on success and 1 on failure*/
    assert(fprintf(stderr, "check_SB()\n"));
    uint32_t file = f->fd;
    off_t offset = f->offset;

    offset += SBOFFSET;

    if(-1 == lseek(file, offset, SEEK_SET)){ /*seek to start of super block*/
      perror("lseek");
      return 1;
    }

    if(-1 == read(file, s, sizeof(struct superblock))){
        perror("read");
        return 1;
    }

    if(s->magic != MAGIC){
      fprintf(stderr, "Not a valid filesystem (Magic mismatch)\n");
      return 1;
   }

   logzonesize(s, f);
   f->indirect = calloc((s->blocksize), sizeof(int32_t));
   f->two_indirect = calloc((s->blocksize), sizeof(int32_t));

    return 0;
}
/* pizza at 6:36 */
/*ONLY CALL THIS ONCE*/
int fill_root_ino(finder *f, superblock *s, inode_minix *i){
   assert(fprintf(stderr, "fill_root_ino()\n"));
   uint32_t file = f->fd;
   uint32_t imap_size = 0;
   uint32_t zmap_size = 0;
   /*set offset to 2 blocks more. Gets us to imap*/
   f->offset += (2*(s->blocksize));

   /*calculate size of imap and add to offset*/
   imap_size = s->i_blocks * s->blocksize;
   f->offset += imap_size;

   /*calculate size of zmap and add to offset*/
   zmap_size = s->z_blocks * s->blocksize;
   f->offset += zmap_size;

   /*should be at beginging of inode table, fill root inode*/
   if(-1 == lseek(file, f->offset, SEEK_SET)){ /*seek to start of inode table*/
     perror("lseek");
     return 1;
   }

   if(-1 == read(file, i, sizeof(struct inode_minix))){
      perror("read");
      return 1;
   }

   return 0;
}

/* for now it's self explanitory */
int get_type(parser *p, inode_minix *i){
   //assert(fprintf(stderr, "get_type()\n"));
   if(i->links == 0){
      return -1;
   }
   switch(i->mode & F_TYPE_MASK){
      case REG_FILE:
         return 0;
      break;

      case DIRECTORY:
         return 1;
      break;

      default:
         printf("%s is not a standard file or folder\n", p->current);
         return -1;
      break;
   }
}





/* returns 0 on success an 1 on failure with error message*/
int find_target(superblock *s, finder *f, parser *p, inode_minix *i){
   assert(fprintf(stderr, "find_target()\n"));
   int run = YES, last = NO, brk = NO;
   size_t zone;
   int32_t loopVar = 7;
   /*zone represents the offset to the next zone you're looking for.*/
   if(NULL == memcpy(&(f->target),i,INO_SIZE)){
      perror("memcpy()");
      return 1;
   }

   if((NULL != (p->srcpath))){
      assert(fprintf(stderr, "srcpath is not NULL!!!\n"));
      run = next_name(p);
      if(!run){
         last = YES;
      }
   } else{

   /* update target inode */

         return 0;
   }
   /* if it is a file or folder, proceed */
   do{
/***************************DIRECT ZONES***************************************/
      for(int k = 0; k < loopVar; k++){
         /* move on to next zone if empty */
         if(!(f->target.zone[k])){
            assert(fprintf(stderr, "zone # %d is blank\n",k));
            continue;
         }
         /* find blocks per zone, multiply by zone number */
         if(!(zone = seek_zone(f->target.zone[k], f->zonesize, f->last_sector,
            f->fd))){
            assert(fprintf(stderr, "DAVID shit the bed!!!!!!!!!!!!!!!!!!\n"));
            return 1;
         }
         /* for each zone, run through and look at each dir etry */
         for(uint32_t j = 0; j < (f->target.size)/DIR_SIZE; j++){
            /* seek to  */
            lseek(f->fd, f->partoff + zone, SEEK_SET);
            zone += read( f->fd, &(f->dir_ent), sizeof(struct dir_entry));
            /* coppying the current name into nulltermed array of 61 for easy
            comparison */
            memcpy( (p->compare), &(f->dir_ent.name), sizeof(p->compare)-1 );
            /*assert(fprintf(stderr,"current = %s compare = %s DIRECT LOOP#
            %d:%d\n",p->current,p->compare,k,j));*/
            /* found current name in path */
            if(!strcmp( p->current, p->compare )){
               assert(fprintf(stderr, "************MATCH**************\n"));
               assert(fprintf(stderr,"current = %s compare = %s\n",
               p->current,p->compare));
               /*seek to offset of root inode + ((the inode #)X(inode size))*/
               lseek(f->fd, f->offset+((f->dir_ent.inode-1)*INO_SIZE),
               SEEK_SET);
               /* update target inode */
               read(f->fd, &(f->target), INO_SIZE);
               assert(fprintf(stderr, "target inode %d and it's size %u\n",
               f->dir_ent.inode, f->target.size));
               /*target found and last name in path*/
               if(!last){  /*If last is NO*/
                  assert(fprintf(stderr, "current= %s, next_name returned %d\n",
                  p->current,run));
                  if( !(run = next_name(p))){
                     assert(fprintf(stderr,"WE WENT IN\n"));
                     last = YES;
                  }
                  brk = YES;
                  break;
               }
               else{
                  return 0;
               }
            }

         }
         if(brk){
            break;
         }
      }
      if(brk){
         brk = NO;
         continue;
      }



/*****************************INDIRECT ZONES***********************************/
      if(brk){
         brk = NO;
         continue;
      }
      loopVar = (s->blocksize)/4;
      for(int k = 0; k < loopVar; k++){
         /* move on to next zone if empty */
         if(!(f->target.zone[k])){
            assert(fprintf(stderr, "zone # %d is blank\n",k));
            continue;
         }
         /* find blocks per zone, multiply by zone number */
         if(!(zone = seek_zone(f->target.zone[k], f->zonesize, f->last_sector,
            f->fd))){
            assert(fprintf(stderr, "DAVID shit the bed!!!!!!!!!!!!!!!!!!!!\n"));
            return 1;
         }
         /* for each zone, run through and look at each dir etry */
         for(uint32_t j = 0; j < (f->target.size)/DIR_SIZE; j++){
            /* seek to  */
            lseek(f->fd, f->partoff + zone, SEEK_SET);
            zone += read( f->fd, &(f->dir_ent), sizeof(struct dir_entry));
            /* coppying the current name into nulltermed array of 61 for easy
            comparison */
            memcpy( (p->compare), &(f->dir_ent.name), sizeof(p->compare)-1 );
            /*assert(fprintf(stderr,"current = %s compare = %s INDERECT LOOP#
            %d:%d\n",p->current,p->compare,k,j));*/
            /* found current name in path */
            if(!strcmp( p->current, p->compare )){
               assert(fprintf(stderr, "*************MATCH***************\n"));
               assert(fprintf(stderr,"current = %s compare = %s\n",p->current,
               p->compare));
               /*seek to offset of root inode + ((the inode #)X(inode size)) */
               lseek(f->fd, f->offset+((f->dir_ent.inode-1)*INO_SIZE),
               SEEK_SET);
               /* update target inode */
               read(f->fd, &(f->target), INO_SIZE);
               assert(fprintf(stderr, "target= %d and it's size  %u\n",
               f->dir_ent.inode, f->target.size));
               /*target found and last name in path*/
               if(!last){
                  assert(fprintf(stderr,"current= %s, next_name returned %d\n",
                  p->current,run));
                  if( !(run = next_name(p)) )
                     last = YES;
                  brk = YES;
                  break;
               }
               else
                  return 0;
            }

         }
         if(brk){
            break;
         }
      }

      if(brk){
         brk = NO;
         continue;
      }

/***************************DOUBLE INDIRECT ZONES******************************/
      if(brk){
         brk = NO;
         continue;
      }
      for(int j = 0; j < loopVar; j++){
         fill_indirect(*(f->two_indirect + j), s, f);
         for(int k = 0; k < loopVar; k++){
            /* move on to next zone if empty */
            if(!(*(f->indirect + k))){
               assert(fprintf(stderr, "zone # %d is blank\n",k));
               continue;
            }
            if(!(zone = seek_zone((*(f->indirect+k)), f->zonesize,
            f->last_sector, f->fd))){
               assert(fprintf(stderr, "DAVID shit the bed!!!!!!!!!!!\n"));
               return 1;
            }
            /* for each zone, run through and look at each dir etry */
            for(uint32_t l = 0; l < (f->target.size)/DIR_SIZE; l++){
               /* seek to  */
               lseek(f->fd, f->partoff + zone, SEEK_SET);
               zone += read( f->fd, &(f->dir_ent), sizeof(struct dir_entry));
               /* coppying the current name into nulltermed array of 61 for
               easy comparison */
               memcpy( (p->compare), &(f->dir_ent.name), sizeof(p->compare)-1 );
               /*assert(fprintf(stderr,"current = %s compare = %s DOUBLE-
               INDERECT LOOP# %d:%d:%d\n",p->current,p->compare,j,k,l));*/
               /* found current name in path */
               if(!strcmp( p->current, p->compare )){
                  assert(fprintf(stderr, "**************MATCH***********\n"));
                  assert(fprintf(stderr,"current = %s compare = %s\n",
                  p->current,p->compare));
                  /* offset of root inode + ((the inode #)X(inode size)) */
                  lseek(f->fd, f->offset+((f->dir_ent.inode-1)*INO_SIZE),
                  SEEK_SET);
                  /* update target inode */
                  read(f->fd, &(f->target), INO_SIZE);
                  assert(fprintf(stderr, "target %d and it's size is %u\n",
                  f->dir_ent.inode, f->target.size));
                  /*target found and last name in path*/
                  if(!last){
                     assert(fprintf(stderr,"curr %s, next_name returned %d\n",
                     p->current,run));
                     if(!(run = next_name(p))){
                        last = YES;
                     }
                     break;
                  }
                  else
                     return 0;
               }
            }
         }
      }
   }while(run || last);
   return 1;
}


/*finds and checks if zonesize is valid*/
uint32_t seek_zone(uint32_t zone_num, uint32_t zone_size, uint32_t last_sector,
    int32_t fd){
   //if(zone_num != 0)
      //assert(fprintf(stderr, "seek_zone()  zone_num: %u\n", zone_num));
   /*returns -1 if out of bounds*/
   uint32_t where, cutoff;

   if(last_sector){
      cutoff = (last_sector+1)*512;
   }
   else{
      cutoff = lseek(fd, 0, SEEK_END);
   }
   //assert(fprintf(stderr, "seek_zone()  cutoff: %u\n", cutoff));
   if((where = zone_num * zone_size) > cutoff){
      //fprintf(stderr, "Zone is out of bounds. Zone: %u\n", where);
       return 0;
   }
   //assert(fprintf(stderr, "seek_zone()  zone_num: %u\n", zone_num));
   return where;
}



/* "All directories are linked into a tree starting
   at the root directory at inode 1." ....         ????????     */


/* returns 1 if there's an additional name, -1 if name > 60, and 0 when done */
int next_name(parser *p){
   assert(fprintf(stderr, "next_name()\n"));
   assert(fprintf(stderr, "path is %s\n", p->srcpath));
   static int place = 0;
   int i, c, size, where = 0;
   size =  strlen(p->srcpath);
   /* skip first '/' if there is one*/
   if(  (place == 0) && ( (p->srcpath)[0] == '/') )
      place = 1;
   assert(fprintf(stderr, "next_name palce = %d\n",place));
   /* traversing path statically returning each time a '/' is encountered */
   for(i = place; i < size; i++){
      c = (p->srcpath)[i];
      assert(fprintf(stderr, "i= %d, c= %c, size= %d, place= %d, where= %d\n",
                              i,     c,     size,     place,     where));
      if(c == '/'){
         place = place + 1;
         p->current[where] = '\0';
         assert(fprintf(stderr, "next_name palce = %d returning 1*\n",place));
         return 1;
      }
      p->current[where] = c;
      if(where > 59){
         assert(fprintf(stderr, "next_name palce = %d returning -1\n",place));
         return -1;
      }
      place++;
      where++;
   }
   p->current[where] = '\0';
   assert(fprintf(stderr, "next_name palce = %d returning 0***\n",place));
   return 0;
}

/* returns 0   ....ehh */
int logzonesize(superblock *s, finder *f){
   assert(fprintf(stderr, "logzonesize()\n"));
   /* left shifting log_zon... into zonesize */
   f->zonesize = (s->blocksize) << s->log_zone_size;
   return 0;
}

void close_file(finder *f){
    close(f->fd);
    free(f->indirect);
    free(f->two_indirect);
}

int parse_line_ls(struct parser *parse, int argc, char **argv){
   assert(fprintf(stderr, "parse_line_ls()\n"));
    int32_t c;
    char *endptr;
    extern char *optarg;
    extern int optind;

    while( (c=getopt(argc, argv, "hvp:s:")) != -1 ){
       switch(c){
            case 'v':
                parse->verbose += 1;
                break;
            case 'p':
                if(parse->partition >= 0){ /*can't have more than one*/
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
                if(parse->sector >= 0){  /*can't have more than one*/
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

/*coppies indirect in finder struct*/
int fill_indirect(int32_t indirect_zone, superblock *s, finder *f){
   /*0 for success 1 for failure*/
   int32_t indirect;
   if(!(indirect = seek_zone(indirect_zone, f->zonesize,
                              f->last_sector, f->fd))){
      return 0;
   }

   if( indirect < 0 ){
      return 1;
   }

   if(indirect_zone){
      if(-1 == lseek(f->fd, indirect, SEEK_SET)){
         perror("lseek");
         return 1;
      }

      if(-1 == read(f->fd, f->indirect, s->blocksize)){
         perror("read");
         return 1;
      }
   }

   return 0;
}

/*copies double indirect in finder struct*/
int fill_two_indirect(int32_t two_indirect_zone, superblock *s, finder *f){
   /*0 for success 1 for failure*/
   int32_t two_indirect;
   if(!(two_indirect = seek_zone(two_indirect_zone, f->zonesize,
                                    f->last_sector, f->fd))){
      return 0;
   }

   if(two_indirect < 0){
      return 1;
   }

   if(two_indirect_zone){
      if(-1 == lseek(f->fd, two_indirect, SEEK_SET)){
         perror("lseek");
         return 1;
      }

      if(-1 == read(f->fd, f->two_indirect, s->blocksize)){
         perror("read");
         return 1;
      }
   }

   return 0;
}

/*define perms as ['-'] * 12 before calling this function*/
int fill_perms(char *perms, int32_t type, mode_t mode){
   //assert(fprintf(stderr, "fill_perms()\n"));
   /*returns 0 on success and 1 on failure*/
   memset(perms, '-', 10);
   /*check if dir, file, or niether*/
   if(type < 0){
      return 1;
   }

   if(type){
      *perms = 'd';
   }

   /*now fill permissions*/
   if(mode & O_RD_PERM){
      *(perms + 1) = 'r';
   }

   if(mode & O_WT_PERM){
      *(perms + 2) = 'w';
   }

   if(mode & O_EX_PERM){
      *(perms + 3) = 'x';
   }

   if(mode & G_RD_PERM){
      *(perms + 4) = 'r';
   }

   if(mode & G_WT_PERM){
      *(perms + 5) = 'w';
   }

   if(mode & G_EX_PERM){
      *(perms + 6) = 'x';
   }

   if(mode & OTH_RD_PERM){
      *(perms + 7) = 'r';
   }

   if(mode & OTH_WT_PERM){
      *(perms + 8) = 'w';
   }

   if(mode & OTH_EX_PERM){
      *(perms + 9) = 'x';
   }

    return 0;
}

int parse_line_get(struct parser *parse, int argc, char **argv){
   assert(fprintf(stderr, "parse_line_get()\n"));
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
                if(parse->partition >= 0){   /*can't have more than one*/
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
                if(parse->sector >= 0){
                    print_usage_get();
                    return 1;
                }
                parse->sector = strtol(optarg, &endptr, 10);
                if(*endptr){
                    print_usage_get();
                    return 1;
                }
                break;

            case 'h':
                print_usage_get();
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

    if(parse->sector != -1 && parse->partition == -1){
        /*can not have subpartition with no partition*/
        print_usage_get();
        return 1;
    }
    return 0;
}

/*this function is to print for minls - returns int to pass message*/
int ls_file(finder *f, parser *p, superblock *s){
   assert(fprintf(stderr, "ls_file()\n"));
   /*0 on success and 1 on failure*/
   uint32_t num_bytes, counter, check, ob, blocksize;
   size_t zone;
   int32_t type;
   inode_minix i, target;
   dir_entry d;
   target = f->target;
   type = get_type(p, &target);
   blocksize = s->blocksize;
   char perms[11] = {'-','-','-','-','-','-','-','-','-','-','\0'};
   num_bytes = target.size;
   counter = 0;
   ob = 0;   /*out of bounds*/
   zone = 0;

   assert(fprintf(stderr, "YUH: %u\n", target.indirect));
   if((check = fill_indirect(target.indirect, s, f))){
       return 1;
   }
   assert(fprintf(stderr, "HE THEREEEE\n"));
   if((check = fill_two_indirect(target.two_indirect, s, f))){
       return 1;
   }

    /*START*/
   if(type < 0){
      return 1;
   }

   if(type){  /*DIRECTORY*/
   assert(fprintf(stderr, "HE THEREEEE\n"));
      if(p->srcpath){
         printf("%s:\n", p->srcpath);
      }
      else{
         printf("/:\n");
      }
      /*FOR DIRECT ZONES*/
      for(int k=0; k < 7 ; k++){
         // if(!(target.zone[k])){
         //    continue;
         // }

         if(!(zone = seek_zone(target.zone[k],f->zonesize,
                                 f->last_sector, f->fd))){
            assert(fprintf(stderr, "Offset: %u\n",f->zonesize));
            return 1;
         }

          /*here counter counts directories*/
         while(counter <= (num_bytes/DIR_SIZE) && ob < (f->zonesize/DIR_SIZE)){
            lseek(f->fd, f->partoff + zone, SEEK_SET);
            /*in correct zone now, traverse dir_ents*/

            zone += read(f->fd, &d, sizeof(dir_entry));/*find next entry*/

            if(d.inode){
               /*go to inode table and needed inode*/
               lseek(f->fd, (f->offset + ((d.inode -1)*INO_SIZE)), SEEK_SET);
               read(f->fd, &i, sizeof(inode_minix));
               /*here's where we do the printing*/
               type = get_type(p, &i);
               check = fill_perms(perms, type, i.mode);
               printf("%s    %6u %s\n", perms, i.size, d.name);
            }
            counter++;
            ob++;
         }
         ob = 0;
         if(counter == (num_bytes/DIR_SIZE)){ /*no need to check more blocks*/
            return 0;
         }
      }

      /*FOR INDIRECT ZONES*/
      for(uint32_t k = 0; k < (blocksize/4) ; k++){
         if(!(*(f->indirect + k))){
            continue;
         }
         if(!(zone = seek_zone(*(f->indirect + k),
                               f->zonesize, f->last_sector, f->fd))){
            assert(fprintf(stderr, "Offset: %lu***************\n",zone));
            return 1;
         }

         /*here counter counts directories*/
         while(counter <= (num_bytes/DIR_SIZE) && ob < (f->zonesize/DIR_SIZE)){
            lseek(f->fd, f->partoff + zone, SEEK_SET);
            /*in correct zone now, traverse dir_ents*/

            zone += read(f->fd, &d, sizeof(dir_entry));/*find next entry*/

            if(d.inode){
               /*go to inode table and needed inode*/
               lseek(f->fd, f->offset + ((d.inode-1)*INO_SIZE), SEEK_SET);
               read(f->fd, &i, sizeof(inode_minix));
               /*here's where we do the printing*/
               type = get_type(p, &i);
               check = fill_perms(perms, type, target.mode);
               printf("%s    %6u %s\n", perms, i.size, d.name);
               memset(perms, '-', 10);
            }
            counter++;
            ob++;
         }
         ob = 0;
         if(counter == (num_bytes/DIR_SIZE)){ /*no need to check more blocks*/
            return 0;
         }
      }
      /*FOR DOUBLE INDIRECT*/
      for(uint32_t l = 0; l < (blocksize/4); l++){
          fill_indirect(*(f->two_indirect + l), s, f);
          /*go through indirect as necesssry*/
          for(uint32_t k = 0; k < (blocksize/4) ; k++){
            if(!(*(f->indirect + k))){
               continue;
             }

             if(!(zone=seek_zone(*(f->indirect+k),f->zonesize,
                                                f->last_sector, f->fd))){
                assert(fprintf(stderr, "Offset: %lu\n",zone));
               return 1;
             }

            /*here counter counts directories*/
            while(counter <= (num_bytes/DIR_SIZE) &&
                                    ob < (f->zonesize/DIR_SIZE)){
                lseek(f->fd, f->partoff + zone, SEEK_SET);
                /*in correct zone now, traverse dir_ents*/

                zone += read(f->fd, &d, sizeof(dir_entry));/*find next entry*/
                /*go to inode table and needed inode*/

                if(d.inode){
                   lseek(f->fd, f->offset + ((d.inode-1)*INO_SIZE), SEEK_SET);
                   read(f->fd, &i, sizeof(inode_minix));
                   /*here's where we do the printing*/
                   type = get_type(p, &i);
                   check = fill_perms(perms, type, target.mode);
                   printf("%s    %6u %s\n", perms, i.size, d.name);
                }
                counter++;
                ob++;
            }
            ob = 0;
            if(counter == (num_bytes/DIR_SIZE)){
               /*no need to check more blocks*/
                return 0;
            }
          }
      }
   }
      /*FOR FILES - target is the file we want, we have the inode*/
   else{
      if(p->srcpath){
         printf("/%s\n", p->current);
      }
      else{
         printf("/:\n");
      }
      check = fill_perms(perms, type, target.mode);
      printf("%s    %6u %s\n", perms, target.size, p->srcpath);
      return 0;
   }
   return 0;
}

int get_file(finder *f, parser *p, superblock *s){
   assert(fprintf(stderr, "get_file()\n"));
   /*0 on success and 1 on failure*/
   uint32_t num_bytes, howmuch, blocksize;
   int32_t zone, type, wfile;
   inode_minix target;
   uint8_t *real_zone;
   blocksize = s->blocksize;

   target = f->target;
   type = get_type(p, &target);
   real_zone = malloc(f->zonesize);

   /*Open the corect */
   if(p->dstpath){
      if(-1 == (wfile = open(p->dstpath, O_WRONLY & O_CREAT & O_TRUNC))){
         free(real_zone);
         return 1;
      }
   }
   else{
      wfile = STDOUT_FILENO;
   }

   /*file size here*/
   num_bytes = target.size;

   /*START*/
  if(type < 0){
     if(p->dstpath)
         close(wfile);
     free(real_zone);
     return 1;
  }

  if(type){  /*DIRECTORY*/
     if(p->srcpath){
         printf("%s: Not a Regular File\n", p->srcpath);
     }
     else{
         printf("/: Not a Regular File\n");
     }

     if(p->dstpath)
         close(wfile);
     free(real_zone);
     return 1;
  }
  else{
  /* if(p->srcpath){
        printf("%s:\n", p->srcpath);
     }
     else{
        printf("/:\n");
     }*/
     /*FOR DIRECT ZONES*/
     for(int k=0; k < 7 ; k++){
        /*calculate how much to write*/
        if(num_bytes <= 0){
           if(p->dstpath)
              close(wfile);
           free(real_zone);
           return 0;
        }
        howmuch = (num_bytes > f->zonesize) ? f->zonesize : num_bytes;
        if(!(target.zone[k])){
            memset(real_zone, 0, f->zonesize);
            /*check this please*/
            num_bytes -= write(wfile, real_zone, howmuch);
            continue;
        }

        if(!(zone = seek_zone(target.zone[k],f->zonesize,
                                    f->last_sector, f->fd))){
           assert(fprintf(stderr, "Offset: %u\n",f->zonesize));
           if(p->dstpath)
              close(wfile);
           free(real_zone);
           return 1;
        }

        /*in correct zone now, traverse dir_ents*/
        lseek(f->fd, f->partoff + zone, SEEK_SET);

        read(f->fd, real_zone, f->zonesize);/*fill block*/

        num_bytes -= write(wfile, real_zone, howmuch);/*writes to output*/
     }

   /*FOR INDIRECT ZONES*/
      for(uint32_t k = 0; k < (blocksize/4) ; k++){
         /*calculate how much to write*/
         if(num_bytes <= 0){
            if(p->dstpath)
               close(wfile);
            free(real_zone);
            return 0;
         }
         howmuch = (num_bytes > f->zonesize) ? f->zonesize : num_bytes;
         if(!(*(f->indirect + k))){
             memset(real_zone, 0, f->zonesize);
             /*check this please*/
             num_bytes -= write(wfile, real_zone, howmuch);
             continue;
         }

         if(!(zone = seek_zone(target.zone[k],f->zonesize,
                                          f->last_sector, f->fd))){
            assert(fprintf(stderr, "Offset: %u\n",f->zonesize));
            if(p->dstpath)
               close(wfile);
            return 1;
         }

         /*in correct zone now, traverse dir_ents*/
         lseek(f->fd, f->partoff + zone, SEEK_SET);

         read(f->fd, real_zone, f->zonesize);/*fill block*/

         num_bytes -= write(wfile, real_zone, howmuch);/*writes to output*/
      }

   /*FOR DOUBLE INDIRECT*/
      for(uint32_t l = 0; l < (blocksize/4); l++){
          fill_indirect(*(f->two_indirect + l), s, f);
          /*go through indirect as necesssry*/
          for(uint32_t k = 0; k < (blocksize/4) ; k++){
             /*calculate how much to write*/
             if(num_bytes <= 0){
                if(p->dstpath)
                   close(wfile);
                free(real_zone);
                return 0;
             }
             howmuch = (num_bytes > f->zonesize) ? f->zonesize : num_bytes;
             if(!(*(f->indirect + k))){
                 memset(real_zone, 0, f->zonesize);
                 /*check this please*/
                 num_bytes -= write(wfile, real_zone, howmuch);
                 continue;
             }

             if(!(zone = seek_zone(target.zone[k],f->zonesize,
                                             f->last_sector, f->fd))){
                assert(fprintf(stderr, "Offset: %u\n",f->zonesize));
                if(p->dstpath)
                   close(wfile);
                return 1;
             }

             /*in correct zone now, traverse dir_ents*/
             lseek(f->fd, f->partoff + zone, SEEK_SET);

             read(f->fd, real_zone, f->zonesize);/*fill block*/

             num_bytes -= write(wfile, real_zone, howmuch);/*writes to output*/
          }
      }
   }
   if(num_bytes <= 0){
      printf("File wayyyyy too big.\n");
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

void verbose0(part_table *t,parser *p,finder *f,superblock *s,inode_minix *i){
   switch(p->verbose){
      case(1):
         verbose1(s, f, i);
      break;

      case(2):
         verbose2(p, f, t, s, i);
      break;

      default:
         break;
   }
}

void verbose1(superblock *s, finder *f, inode_minix *i){
   time_t currtime;
    printf("Superblock Contents:\n");
    printf("\tninodes: %d\n", s->ninodes);
    printf("\ti_blocks: %d\n", s->i_blocks);
    printf("\tz_blocks: %d\n", s->z_blocks);
    printf("\tfirstdata: %d\n", s->firstdata);
    printf("\tlog zone size: %d    (zone size: %u)\n",
                              s->log_zone_size, f->zonesize);
    printf("\tfile size: %u\n", s->max_file);
    printf("\tnum zones: %d\n", s->zones);
    printf("\tmagic num: 0x%X\n", s->magic);
    printf("\tblocksize: %d\n", s->blocksize);
    printf("\tsubversion:%d\n", s->subversion);
    printf("\n");

    printf("File Inode:\n");
    printf("\tuint16_t mode: %X\n", i->mode);
    printf("\tuint16_t links: %u\n", i->links);
    printf("\tuint16_t uid: %u\n", i->uid);
    printf("\tuint16_t gid: %u\n", i->gid);
    printf("\tuint16_t size: %u\n", i->size);
    /*convert times to a string*/
    currtime = (time_t)(i->atime);
    printf("\tuint32_t atime: %u --- %s", i->atime, ctime(&currtime));

    currtime = (time_t)(i->mtime);
    printf("\tuint32_t mtime: %u --- %s", i->mtime, ctime(&currtime));

    currtime = (time_t)(i->atime);
    printf("\tuint32_t ctime: %u --- %s\n", i->ctime, ctime(&currtime));

    printf("\nDirect Zones:\n");
    printf("\t\t\tzone [0] = %u\n", i->zone[0]);
    printf("\t\t\tzone [1] = %u\n", i->zone[1]);
    printf("\t\t\tzone [2] = %u\n", i->zone[2]);
    printf("\t\t\tzone [3] = %u\n", i->zone[3]);
    printf("\t\t\tzone [4] = %u\n", i->zone[4]);
    printf("\t\t\tzone [5] = %u\n", i->zone[5]);
    printf("\t\t\tzone [6] = %u\n", i->zone[6]);
    printf("\tuint32_t indirect = %u\n", i->indirect);
    printf("\tuint32_t double = %u\n", i->two_indirect);
}

void verbose2(parser *p, finder *f, part_table *part, superblock *s,
              inode_minix *i){
    verbose1(s, f, i);
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

    for(int i = 0; i < 4; i++){
        printf("Entry [%d]:\n", i);
        printf("\tBoot Ind: %d\n", part->entry[i].bootind);
        printf("\tStart (Head, Sec, Cyl): %d, %d, %d\n",
                part->entry[i].start_head,
                part->entry[i].start_sec, part->entry[i].start_cyl);
        printf("\tSys Ind: 0x%X\n", part->entry[i].sysind);
        printf("\tLast (Head, Sec, Cyl): %d, %d, %d\n",
                part->entry[i].last_head,
                part->entry[i].last_sec, part->entry[i].last_cyl);
        printf("\tFirt Sector: %d\n", part->entry[i].lowsec);
        printf("\tSize: %d\n", part->entry[i].size);
        printf("\n\n");
    }
}

void print_inode(inode_minix *i){
   time_t currtime;
   printf("File Inode:\n");
   printf("\tuint16_t mode: %X\n", i->mode);
   printf("\tuint16_t links: %u\n", i->links);
   printf("\tuint16_t uid: %u\n", i->uid);
   printf("\tuint16_t gid: %u\n", i->gid);
   printf("\tuint16_t size: %u\n", i->size);
   /*convert times to a string*/
   currtime = (time_t)(i->atime);
   printf("\tuint32_t atime: %u --- %s", i->atime, ctime(&currtime));

   currtime = (time_t)(i->mtime);
   printf("\tuint32_t mtime: %u --- %s", i->mtime, ctime(&currtime));

   currtime = (time_t)(i->atime);
   printf("\tuint32_t ctime: %u --- %s\n", i->ctime, ctime(&currtime));
}


int test_next_name(parser *p){
   /*assert(test_next_name(p));*/
   /* 012345678901234567890123456789012345678901234567890123456789 */
   /* This is a 60 character string to see if the cap is at real . */
   int status;
   while(1){
      status = next_name(p);
      printf("the status is %d\n", status);
      if( status != 0 && status > 0 )
         printf( "next_name returned %s\n", p->current );
      else
         break;
   }
   if(status == 0)
      printf( "next_name returned %s\n", p->current );
   return 1;
}
/* end */

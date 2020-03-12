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

   while(counter <= (num_bytes/DIR_SIZE) && ob < (f->zonesize/DIR_SIZE)){ /*here counter counts directories*/
      lseek(f->fd, f->partoff + zone, SEEK_SET);/*in correct zone now, traverse dir_ents*/
      zone += read(f->fd, &d, sizeof(dir_entry));/*find next entry*/
      /*go to inode table and needed inode*/
      lseek(f->fd, f->offset + ((d.inode)*INO_SIZE), SEEK_SET);
      read(f->fd, &i, sizeof(inode_minix));
      /*here's where we do the printing*/
      type = get_type(p, &i);
      check = fill_perms(perms, type, target.mode);
      printf("%s\t\t%6u %s\n", perms, i.size, d.name);
      memset(perms, '-', 10);
      /*seek back to zone where we left off*/
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
    for(uint32_t k = 0; k < (blocksize/4) ; k++){/*go through indirect as necesssry*/
      if(!(*(f->indirect + k))){
         continue;
      }

      if(!(zone=seek_zone((*(f->indirect+k)),f->zonesize,f->last_sector, f->fd))){
          assert(fprintf(stderr, "Offset: %lu\n",zone));
         return 1;
      }

      while(counter <= (num_bytes/DIR_SIZE) && ob < (f->zonesize/DIR_SIZE)){ /*here counter counts directories*/
          lseek(f->fd, f->partoff + zone, SEEK_SET);/*in correct zone now, traverse dir_ents*/
          zone += read(f->fd, &d, sizeof(dir_entry));/*find next entry*/
          /*go to inode table and needed inode*/

          if(d.inode){
             lseek(f->fd, f->offset + ((d.inode)*INO_SIZE), SEEK_SET);
             read(f->fd, &i, sizeof(inode_minix));
             /*here's where we do the printing*/
             type = get_type(p, &i);
             check = fill_perms(perms, type, target.mode);
             printf("%s\t\t%6u %s\n", perms, i.size, d.name);
          }
          counter++;
          ob++;
      }
      ob = 0;
      if(counter == (num_bytes/DIR_SIZE)){ /*no need to check more blocks*/
          return 0;
      }
    }
}

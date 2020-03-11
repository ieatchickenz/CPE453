#include "min_funcs.h"

int main(int argc, char **argv)
{
   assert(fprintf(stderr, "ASSERTION_ON\n"));
   parser p;
   finder f;
   part_table t;
   inode_minix i, targ;
   superblock s;
   int32_t check;
   init_parser(&p);
   init_finder(&f);
   init_part_table(&t);
   if( (check = parse_line_ls(&p, argc, argv)) ){
      exit(1);
   }

   if( (check = find_filesystem(&p, &f, &t)) ){
      return 1;
   }

   if( (check = check_SB(&f, &s)) ){
      return 1;
   }

   if( 1 == fill_root_ino(&f, &s, &i) ){
      return 1;  /**************instead of returning 1, we should call an
      exit function that will close the passed in file***********????????*****/
   }

   

   find_target(&s, &f, &p, &i);
   
   if(p.verbose){
      verbose0(&t, &p, &f, &s, &(f.target));
   }
   
   ls_file(&f, &p, &s);

   close_file(&f);
   return 0;
}

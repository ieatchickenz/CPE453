#include "min_funcs.h"

int test_next_name(parser p){
   int status;
   while(1){
      status = next_name(&p);
      printf("the status is %d\n", status);
      if( status != 0 && status > 0 )
         printf( "next_name returned %s\n", p.current );
      else
         break;
   }
   if(status == 0)
      printf( "next_name returned %s\n", p.current );
   return 1;
}

int main(int argc, char **argv)
{
   assert(fprintf(stderr, "ASSERTION_ON\n"));
   parser p;
   finder f;
   part_table t;
   inode_minix i;
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

   
   if(p.verbose){
      verbose0(&p, &f, &s, &i);
   }


   assert(test_next_name(p));

   close_file(&f);
   return 0;
}

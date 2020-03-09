#include "min_funcs.h"

int main(int argc, char **argv)
{
   assert(fprintf(stderr, "ASSERTION_ON\n"));
   parser p;
   finder f;
   part_table t;
   inode_minix i;
   superblock s;
   int32_t check;
   int file = 0;
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

   fill_ino(&f, &s, &i);

   switch(p.verbose){
      case(1):
         verbose1(&s, &f, &i);
      break;

      case(2):
         verbose2(&p, &f, &t, &s, &i);
      break;

      default:
         break;
    }

   while( 1 != next_name(&p) )
      printf( "the current name is %s\n", p.current );
   printf( "the current name is %s\n", p.current );
   
   close(file);
   printf("test print since nothing else is doing anything\n");
   return 0;
}

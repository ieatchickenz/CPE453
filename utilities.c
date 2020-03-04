#include "utilities.h"

/*
usage:
   use with assert:
   EXAMPLE:
   assert(test_prints("calling member, what's happening"));
*/
void  exit_util(const char *failure_name){
   perror(failure_name);
   exit(EXIT_FAILURE);
}

void *safe_malloc(size_t size, char *sender, size_t extra){
   void *alloced_mem;
   assert(fprintf(stderr,"MALLOCING... to size %lu\n", size + extra));
   if(NULL == (alloced_mem = malloc(size + extra)))
   {
      char *from = "safe_malloc, from ";
      char *error_mes = malloc(strlen(from) + (strlen(sender) + 1));
      strcpy(error_mes, from);
      strcat(error_mes, sender);
      perror(error_mes);
      free(error_mes);
      exit(EXIT_FAILURE);
   }
   return alloced_mem;
}

void *safe_realloc(void *ptr, size_t new_size, char *sender, size_t extra){
   assert(fprintf(stderr,"REALLOCING... to size %lu\n", (new_size + extra)));
   if(NULL == (ptr = realloc(ptr, (new_size + extra)))){
      char *from = "safe_realloc, from ";
      char *error_mes = malloc(strlen(from) + strlen(sender) + 1);
      strcpy(error_mes, from);
      strcat(error_mes, sender);
      perror(error_mes);
      free(error_mes);
      exit(EXIT_FAILURE);
   }
   return ptr;
}

void *safe_calloc(size_t nmemb, size_t size, char *sender, size_t extra){
   void *alloced_mem;
   assert(fprintf(stderr,"CALLOCING... to size %lu\n", size + extra));
   if( (alloced_mem = calloc(nmemb, (size + extra))) == NULL){
      char *from = "safe_malloc, from ";
      char *error_mes = malloc(strlen(from) + strlen(sender) + 1);
      strcpy(error_mes, from);
      strcat(error_mes, sender);
      perror(error_mes);
      free(error_mes);
      exit(EXIT_FAILURE);
   }
   return alloced_mem;
}

/*will print bits of char or int, specify char by setting ischar = 1*/
int print_bits(int num, int ischar){
   int i, length;
   char* bits;
   if(ischar){
      bits = malloc(sizeof(char)*9);
      length = 8;
   }
   else{
      bits = malloc(sizeof(char)*33);
      length = 32;
   }

   for(i=0; i<length; i++){
      if( (num & (1 << i)) )
         (bits)[(length-1)-i] = '1';
      else
         (bits)[(length-1)-i] = '0';
   }
   (bits)[length] = '\0';
   fprintf(stderr, "%s", bits);
   return 1;
}

int print_spaces(int i){
   for(int j=0; j<i; i++)
      printf(" ");
   return 1;
}

void mem_off_wrt(char *strg, int offset, const char *insert){
   assert(fprintf(stderr, "mem_off_wrt()\n"));
   //assert(fprintf(stderr, "\"%s\" and \"%s\"\n", strg, insert));
   for(int i=offset; i<(strlen(insert)+offset); i++)
      strg[i] = insert[i-offset];
}

void print_at(char* strg, int quan){
   assert(fprintf(stderr, "print_at()\n"));
   for(int i=0; i<quan; i++)
      printf("%c", strg[i]);
   printf("\n");
}

char* remove_last_slash(const char* input){
   char* new = malloc((strlen(input))*sizeof(char));
   assert(fprintf(stderr, "remove_last_slash()\n"));
   memcpy(new, input, strlen(input));
   new[strlen(input)-1] = '\0';
   assert(fprintf(stderr, "old path/name = %s\nnew path/name = %s\n", input, new));
   return new;
}

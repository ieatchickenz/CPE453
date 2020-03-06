#ifndef UTILITIES_H
#define UTILITIES_H
#define ALL_PERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <sys/wait.h>
#include <getopt.h>
void  exit_util(const char *failure_name); /*, __FILE__, __LINE__*/
void *safe_malloc(size_t size, char *sender, size_t extra);
void *safe_realloc(void *ptr, size_t new_size, char *sender, size_t extra);
void *safe_calloc(size_t nmemb, size_t size, char *sender, size_t extra);
int   print_bits(int num, int ischar);
int   print_spaces(int i);
void  mem_off_wrt(char* head, int offset, const char *insert);
void  print_at(char* strg, int quan);
char* remove_last_slash(const char* input);
#endif

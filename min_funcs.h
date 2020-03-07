/*  the following is to be defined
    Block size: in superblock   *** what?
    Zone size: k × blocksize
    */
#define SEC_SIZE  512      /* minix sector size        */
#define SBOFFSET  1024     /* offset to Super Block or 0x400*/
#define PARTS     0x1BE    /* partition table location */
#define PTSIZE    128      /*size of the partition table is 128 bytes*/
#define MINIX     0x81     /* minix compatable type #  */
#define SIG1      0x55     /* first sinature for valid partition */
#define SIG2      0xAA     /* second sinature for valid partition */
#define SIG1LOC   510      /* first sinature location for valid partition */
#define SIG2LOC   511      /* second sinature location for valid partition */
#define MAGIC     0x4D5A   /* the minix magic number */
#define MAGIC_RES 0x5A4D   /* minix magic number on byte-reversed filesystem*/
#define INO_SIZE  64       /* size of an inode in bytes */
#define DIR_SIZE  64       /* size of a directory entry in bytes */
#define DIRECT_ZONES 7     /* no fucking clue */

/* from /usr/include/i386/partition.h */
#define ACTIVE_FLAG	0x80	/* value for active in bootind field (hd0) */
#define NR_PARTITIONS	4	/* number of entries in partition table */
#define	PART_TABLE_OFF	0x1BE	/* offset of partition table in boot sector */

/* Partition types. */
#define NO_PART		0x00	/* unused entry */
#define MINIX_PART	0x81	/* Minix partition type */

#define F_TYPE_MASK  0170000
#define REG_FILE     0100000
#define DIRECTORY    0040000
#define O_RD_PERM    0000400
#define O_WT_PERM    0000200
#define O_EX_PERM    0000100
#define G_RD_PERM    0000040
#define G_WT_PERM    0000020
#define G_EX_PERM    0000010
#define OTH_RD_PERM  0000004
#define OTH_WT_PERM  0000002
#define OTH_EX_PERM  0000001

#include "utilities.h"
#include <getopt.h>
/*uint32_t inode inode number
unsigned char name[60] filename string*/

/* Minix Version 3 Superblock this structure found in fs/super.h in minix
 * 3.1.1 on disk. These fields and orientation are non–negotiable */
typedef struct __attribute__((__packed__)) superblock {
  uint32_t ninodes;       /* number of inodes in this filesystem */
  uint16_t pad1;          /* make things line up properly */
  int16_t i_blocks;       /* # of blocks used by inode bit map */
  int16_t z_blocks;       /* # of blocks used by zone bit map */
  uint16_t firstdata;     /* number of first data zone */
  int16_t log_zone_size;  /* log2 of blocks per zone */
  int16_t pad2;           /* make things line up again */
  uint32_t max_file;      /* maximum file size */
  uint32_t zones;         /* number of zones on disk */
  int16_t magic;          /* magic number */
  int16_t pad3;           /* make things line up again */
  uint16_t blocksize;     /* block size in bytes */
  uint8_t subversion;     /* filesystem sub–version */
} superblock;

typedef struct __attribute__((__packed__)) inode {
  uint16_t mode;          /* mode */
  uint16_t links;         /* number or links */
  uint16_t uid;
  uint16_t gid;
  uint32_t size;
  int32_t atime;
  int32_t mtime;
  int32_t ctime;
  uint32_t zone[DIRECT_ZONES];
  uint32_t indirect;
  uint32_t two_indirect;
  uint32_t unused;
} inode;

typedef struct __attribute__((__packed__)) part_entry {
  unsigned char bootind;      /* boot indicator 0/ACTIVE_FLAG	 */
  unsigned char start_head;   /* head value for first sector	 */
  unsigned char start_sec;	   /* sector value + cyl bits for first sector */
  unsigned char start_cyl;	   /* track value for first sector	 */
  unsigned char sysind;       /* system indicator		 */
  unsigned char last_head;    /* head value for last sector	 */
  unsigned char last_sec;     /* sector value + cyl bits for last sector */
  unsigned char last_cyl;     /* track value for last sector	 */
  unsigned long lowsec;       /* logical first sector		 */
  unsigned long size;         /* size of partition in sectors	 */
} part_entry;

typedef struct __attribute__((__packed__)) part_table {
   part_entry entry[4];
} part_table;

typedef struct parser {
    int32_t partition;
    int32_t sector;
    uint32_t verbose;
    char    *imagefile;
    char    *srcpath; /*used for path in minls*/
    char    *dstpath;
} parser;

typedef struct finder{
    off_t offset;
    uint32_t fd;
} finder;

/* initialize parser structure */
void init_parser(parser *p);
/* check the disk image for valid partition table(s), if requested */
uint32_t check_part(parser *p, int file, part_table *part);
/*find the new offset*/
off_t find_offset(int32_t which, struct part_table *part)
/* check for a valid Minix superblock */
int check_SB();
/* check that directories being listed really are directories */
int check_DIR();
/* check that files being copied really are regular files */
int check_file();
/* converts LBA = (c · H + h) · S + s − 1    **** not needed*/
int LBA_convert();
/* calculates actual log zone size zonesize = blocksize << log2 zonesize */
int logzonesize();

void print_usage_ls();

void print_usage_get();

void openfile(struct parser *p, struct finder *f);

int parse_line_ls(struct parser *parse, int argc, char **argv);

int parse_line_get(struct parser *parese, int argc, char **argv);

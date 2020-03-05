/*  the following is to be defined
    Block size: in superblock   *** what?
    Zone size: k × blocksize
    */
#define SEC_SIZE    512   /* minix sector size        */
#define SBOFFSET 1024     /* offset to Super Block */
#define MBR    0x1BE      /* partition table location */
#define MINIX  0x81       /* minix compatable type #  */
#define SIG1 0x55         /* first sinature for valid partition */
#define SIG2 0xAA         /* second sinature for valid partition */
#define SIG1LOC 510       /* first sinature location for valid partition */
#define SIG2LOC 511       /* second sinature location for valid partition */
#define MAGIC 0x4D5A      /* the minix magic number */
#define MAGIC_RES 0x5A4D  /* minix magic number on a byte-reversed filesystem */
#define INO_SIZE 64       /* size of an inode in bytes */
#define DIR_SIZE 64       /* size of a directory entry in bytes */
#define DIRECT_ZONES 7    /* no fucking clue */
#define FILE_TYPE_MASK 0170000
#define REGULAR_FILE 0100000
#define DIRECTORY 0040000
#define OWNER_READ_PERMISSION 0000400
#define OWNER_WRITE_PERMISSION 0000200
#define OWNER_EXECUTE_PERMISSION 0000100
#define GROUP_READ_PERMISSION 0000040
#define GROUP_WRITE_PERMISSION 0000020 
#define GROUP_EXECUTE_PERMISSION 0000010
#define OTHER_READ_PERMISSION 0000004
#define OTHER_WRITE_PERMISSION 0000002
#define OTHER_EXECUTE_PERMISSION 0000001


/*uint32_t inode inode number
unsigned char name[60] filename string*/
/*char usage[] = "usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]
Options:
-p part --- select partition for filesystem (default: none)
-s sub --- select subpartition for filesystem (default: none)
-h help --- print usage information and exit
-v verbose --- increase verbosity level"*/
/*
 * Minix Version 3 Superblock this structure found in fs/super.h in minix 3.1.1
 * on disk. These fields and orientation are non–negotiable
 */
struct superblock {
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
};

struct inode {
  uint16_t mode;          /* mode */
  uint16_t links;         /* number or links */
  uint16_t uid;
  uint16_t gid;
  uint32_t size;
  int32_t atime;
  int32_t mtime;
  int32_t ctime;
  uint32_t zone[DIRECT ZONES];
  uint32_t indirect;
  uint32_t two indirect;
  uint32_t unused;
};

/* check the disk image for valid partition table(s), if partitioning is requested */
int check_part();

/* check for a valid Minix superblock */
int check_SB();

/* check that directories being listed really are directories */
int check_DIR();

/* check that files being copied really are regular files */
int chack_file();

/* converts LBA = (c · H + h) · S + s − 1 */
int LBA_convert();

/* calculates actual log zone size zonesize = blocksize << log2 zonesize */
int logzonesize();












/* */

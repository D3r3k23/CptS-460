/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

// type.h file

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define VA(x) ((u32)x + 0x80000000)
#define PA(x) ((u32)x - 0x80000000)

#define UART0_BASE_ADDR 0x101f1000
#define UART0_DR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x000)))
#define UART0_FR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x018)))
#define UART0_IMSC (*((volatile u32 *)(UART0_BASE_ADDR + 0x038)))

#define UART1_BASE_ADDR 0x101f2000
#define UART1_DR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x000)))
#define UART1_FR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x018)))
#define UART1_IMSC (*((volatile u32 *)(UART1_BASE_ADDR + 0x038)))

#define KBD_BASE_ADDR 0x10006000
#define KBD_CR (*((volatile u32 *)(KBD_BASE_ADDR + 0x000)))
#define KBD_DR (*((volatile u32 *)(KBD_BASE_ADDR + 0x008)))

#define TIMER0_BASE_ADDR 0x101E2000
#define TIMER0_LR (*((volatile u32 *)(UART0_BASE_ADDR + 0x000)))
#define TIMER0_BR (*((volatile u32 *)(UART0_BASE_ADDR + 0x032)))

#define VIC_BASE_ADDR 0x10140000
#define VIC_STATUS    (*((volatile u32 *)(VIC_BASE_ADDR + 0x000)))
#define VIC_INTENABLE (*((volatile u32 *)(VIC_BASE_ADDR + 0x010)))
#define VIC_VADDR     (*((volatile u32 *)(VIC_BASE_ADDR + 0x030)))

#define SIC_BASE_ADDR 0x10003000
#define SIC_STATUS    (*((volatile u32 *)(SIC_BASE_ADDR + 0x000)))
#define SIC_INTENABLE (*((volatile u32 *)(SIC_BASE_ADDR + 0x008)))
#define SIC_ENSET     (*((volatile u32 *)(SIC_BASE_ADDR + 0x008)))
#define SIC_PICENSET  (*((volatile u32 *)(SIC_BASE_ADDR + 0x020)))

#define BLOCK_SIZE 1024
#define BLKSIZE    1024
/*
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef unsigned short ushort;
typedef unsigned long  ulong;
*/
typedef struct ext2_super_block {
	u32	s_inodes_count;		/* Inodes count */
	u32	s_blocks_count;		/* Blocks count */
	u32	s_r_blocks_count;	/* Reserved blocks count */
	u32	s_free_blocks_count;	/* Free blocks count */
	u32	s_free_inodes_count;	/* Free inodes count */
	u32	s_first_data_block;	/* First Data Block */
	u32	s_log_block_size;	/* Block size */
	u32	s_log_frag_size;	/* Fragment size */
	u32	s_blocks_per_group;	/* # Blocks per group */
	u32	s_frags_per_group;	/* # Fragments per group */
	u32	s_inodes_per_group;	/* # Inodes per group */
	u32	s_mtime;		/* Mount time */
	u32	s_wtime;		/* Write time */
	u16	s_mnt_count;		/* Mount count */
	u16	s_max_mnt_count;	/* Maximal mount count */
	u16	s_magic;		/* Magic signature */
	u16	s_state;		/* File system state */
	u16	s_errors;		/* Behaviour when detecting errors */
	u16	s_minor_rev_level; 	/* minor revision level */
	u32	s_lastcheck;		/* time of last check */
	u32	s_checkinterval;	/* max. time between checks */
	u32	s_creator_os;		/* OS */
	u32	s_rev_level;		/* Revision level */
	u16	s_def_resuid;		/* Default uid for reserved blocks */
	u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32	s_first_ino; 		/* First non-reserved inode */
	u16     s_inode_size; 		/* size of inode structure */
	u16	s_block_group_nr; 	/* block group # of this superblock */
	u32	s_feature_compat; 	/* compatible feature set */
	u32	s_feature_incompat; 	/* incompatible feature set */
	u32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	u8	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	u32	s_reserved[206];	/* Padding to the end of the block */
}SUPER;

typedef struct ext2_group_desc
{
	u32	bg_block_bitmap;	/* Blocks bitmap block */
	u32	bg_inode_bitmap;	/* Inodes bitmap block */
	u32	bg_inode_table;		/* Inodes table block */
	u16	bg_free_blocks_count;	/* Free blocks count */
	u16	bg_free_inodes_count;	/* Free inodes count */
	u16	bg_used_dirs_count;	/* Directories count */
	u16	bg_pad;
	u32	bg_reserved[3];
}GD;

typedef struct ext2_inode {
	u16	i_mode;		/* File mode */
	u16	i_uid;		/* Owner Uid */
	u32	i_size;		/* Size in bytes */
	u32	i_atime;	/* Access time */
	u32	i_ctime;	/* Creation time */
	u32	i_mtime;	/* Modification time */
	u32	i_dtime;	/* Deletion Time */
	u16	i_gid;		/* Group Id */
	u16	i_links_count;	/* Links count */
	u32	i_blocks;	/* Blocks count */
	u32	i_flags;	/* File flags */
        u32     dummy;
	u32	i_block[15];    /* Pointers to blocks */
        u32     pad[5];         /* inode size MUST be 128 bytes */
        u32	i_date;         /* MTX date */
	u32	i_time;         /* MTX time */
}INODE;

typedef struct ext2_dir_entry_2 {
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u8	name_len;		/* Name length */
	u8	file_type;
	char	name[255];      	/* File name */
}DIR;


/* Default dir and regulsr file modes */
#define DIR_MODE          0040777 
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0


#define LINES  4
#define N_SCAN 64

#define BLUE   0
#define GREEN  1
#define CYAN   2
#define YELLOW 3
#define PURPLE 4
#define WHITE  5
#define RED    6

#define  SSIZE   1024
#define  NPROC     64
#define  NTHREAD   32
#define  NSIG      32

#define  FREE   0
#define  READY  1
#define  SLEEP  2
#define  BLOCK  3
#define  ZOMBIE 4
#define  PAUSE  5
#define  printf  kprintf

typedef struct semaphore{
  int value;
  struct proc *queue;
}SEMAPHORE;

#define NMINODES          20
#define NMOUNT             4
#define NFD               10
#define NOFT              40
#define NPIPE             10
#define PSIZE            128

#define READ       0
#define WRITE      1
#define RW         2
#define APPEND     3
#define READ_PIPE  4
#define WRITE_PIPE 5

// UNIX <fcntl.h> constants: <asm/fcntl.h> in Linux
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000

typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  struct pipe *pipe_ptr;
  u32  offset;
  char  name[32];
} OFT;

typedef struct pipe{
  char  buf[PSIZE];
  int   head, tail, data, room;
  int   nreader, nwriter;
  int   busy;
}PIPE;
 
/******************** KCW on MINODE *******************
 refCount = # of procs using this MINODE
 lock is a semaphore for exclusive access to this MINODE
              WHY lock? 
 When a proc issues disk I/O on a MINODE, it may gets 
 blocked while waiting for disk completion interrupt =>
 another proc may find the same MINODE and proceed to 
 use it (before fully loaded) OR (even worse!) modify it.
 Hence the need of a lock.
 Unix uses a "locked" flag to sleep/wakeup. 
 MTX uses a semaphore in each MINODE
*****************************************************/
typedef struct Minode{		
  INODE    INODE; 
  int      dev, ino;
  int      refCount;
  int      dirty;
  int      mounted;
  struct Mount *mountptr;
  struct semaphore lock;  // locking semaphore
} MINODE;

typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        struct Minode *mounted_inode;
        char   name[32]; 
        char   mount_name[32];
        // mounted dev's map & inodes_block numbers
        // although all EXT2 FS, these values may be different PER device
        int    BMAP,IMAP,IBLOCK; 
} MOUNT;


typedef struct stat {
  u16 st_dev;		/* major/minor device number */
  u16 st_ino;		/* i-node number */
  u16 st_mode;		/* file mode, protection bits, etc. */
  u16 st_nlink;		/* # links; TEMPORARY HACK: should be nlink_t*/
  u16 st_uid;			/* uid of the file's owner */
  u16 st_gid;		/* gid; TEMPORARY HACK: should be gid_t */
  u16 st_rdev;
  u32   st_size;		/* file size */
  u32   st_atime;		/* time of last access */
  u32   st_mtime;		// time of last modification
  u32   st_ctime;		// time of creation
  u32   st_dtime;
  u32   st_date;
  u32   st_time;
} STAT;

#define PROCESS 0
#define THREAD  1

typedef struct pres{
        int     uid;
        int     gid;
        u32     paddress, size;   // image size in KB
        u32     *pgdir;           // per proc page_dir pointer
        u32     *new_pgdir;       // new_pgdir during exec with new size

        MINODE *cwd;              // CWD 
        char    name[32];         // name string 
        char    tty[32];          // opened /dev/ttyXX
  //int     tcount;           // process tcount
        u32     signal;           // 15 signals=bits 1 to 14   
        int     sig[NSIG];        // 15 signal handlers
        OFT     *fd[NFD];         // open file descriptors
  struct semaphore mlock;         // message passing   
  struct semaphore message;
  struct mbuf     *mqueue;
} PRES;

typedef struct proc{
  struct proc *next;

  int    *ksp;     // at 4
  int    *usp;     // at 8 : Umode sp at time of syscall
  int    *upc;     // at 12: linkR at time of syscall
  int    *cpsr;    // at 16
  int    inkmode;  // at 20
  int    pid;      // at 24

  int    status;
  int    priority;
  //  int    pid;
  int    ppid;
  int tcount;
  
  int    event;
  int    exitCode;
  int    vforked;    // whether the proc is VFROKED 
  int    time;       // time slice  
  int    cpu;        // CPU time ticks used in ONE second
  int    type;       // PROCESS or THREAD
  int    pause;
  int dummy;
  struct proc *parent;
  struct proc  *proc;      // process ptr
  struct pres  *res;       // per process resource pointer 
  struct semaphore *sem;   // ptr to semaphore currently BLOCKed on
  int    kstack[SSIZE];
}PROC;

/**********************************************************************
pgdir of PROC in ARM: 
initial plan: each PROC has a dedicated pgdir at 6M or 7MB by pid
in 7MB: 4KB for each pgdir ==> has space for 1m/4K= 256 pgdirs
defined as pgdir[256], for P0 to P255
P0: pgdir = initial PGDIR= low  2048 entries map 0-2G to PA(0-2G)
                           high 2048 entries map 2G-4G to PA(0-memSize)
Each Pi>0: pgdir = [
                    low entry 0:  map VA(0-1M) to its PA at 8MB+(pid-1)1B
                    high entires: map VA(2G-mem) to (0-mem) for kernel access
                   ]
During task switch: must switch to next running's pgdir (and flush TLB)
**********************************************************************/

// Define devtab[4], but only use devtab[3] to be consistent with PMTX Intel x86
// in buffer.c: initialize all to 0 ==> start_sector = 0 for HD=3
#define NDEV                  4
#define NMBUF              NPROC
// Since HD.partition2 may be in use, define FD as NDEV-1; NOT used in ARM
#define FD  NDEV-1

struct devtab{
  int dev;
  u32  start_sector;
  u32  size;
  struct buf *dev_list;
  struct buf *io_queue;
};

struct hd {		// NOT used in ARM since NO HD drive yet
  u16   opcode;		
  u16   drive;	
  u32    start_sector;
  u32    size;               // size in number of sectors
};

struct floppy {		/* drive struct, one per drive */
  u16 opcode;	/* FDC_READ, FDC_WRITE or FDC_FORMAT */
  u16 cylinder;	/* cylinder number addressed */
  u16 sector;	/* sector : counts from 1 NOT from 0 */
  u16 head;	/* head number addressed */
  u16 count;	/* byte count (BLOCK_SIZE) */
  u32 address;	/* virtual data address in memory */

  char results[7];	/* each cmd may yield upto 7 result bytes */
  char calibration;	/* CALIBRATED or UNCALIBRATED */
  u16  curcyl;	/* current cylinder */
};

// try 4 buffers, each with a 1KB data area
#define NBUF 4

struct buf{
  struct buf *next_free;
  struct buf *next_dev;
  int opcode;             // READ | WRITE
  int dev,blk;

  /********* these status variables could be changed to bits **********/  
  int dirty;
  int busy;
  int async;              //ASYNC write flag
  int valid;              // data valid
  int wanted;

  struct semaphore lock;
  struct semaphore iodone;

  char buf[1024];         // data area
  //  char *buf;            // allocated in binit() to 32 1KB areas
};

struct devsw {
  int (*dev_read)();
  int (*dev_write)();
};

typedef struct mbuf{
  struct mbuf *next;
  int sender;
  char contents[128];
} MBUF;

typedef struct ps{ // 8+16=24 bytes for ps syscall
  u16 pid,ppid,uid,status;
  char pgm[16];
}PS;

extern int boot_dev, HD;
extern int color;
extern char *tab;

extern PROC *kfork();
extern PROC proc[ ], *freeList, *readyQueue, *sleepList, *running;
extern int procsize;
extern OFT  oft[NOFT];
extern PIPE pipe[NPIPE];

extern u32 *MTABLE;
extern int kstrcpy();
extern PROC *getproc();
extern int putproc();
extern int enqueue();
extern PROC *dequeue();
extern int printQ();
extern int printSleepList();
extern int printList();

// uart.c file

#define DR      0
#define SR      4
#define FR     24
#define BUFLEN 64

typedef volatile struct uart{
  char *base;

   // input buffer 
   char inbuf[BUFLEN];
   int inhead, intail;
  struct semaphore inchar;

   // output buffer 
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outspace;
   int tx_on;
   
   // echo buffer 
   char ebuf[BUFLEN];
   int ehead, etail, e_count;


}UART;

extern volatile UART uart[4];    // 4 UART pointers to their base addresses
extern volatile UART *up;

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

// vid.c file
extern u8 cursor;
extern int volatile *fb;
extern unsigned char *font;
extern int row, col;
// fs files


extern MINODE *root; //, *cwd;
//extern char pathname[64], char parameter[64], char *name[32];
extern char names[16][64];
extern int  nnames;
extern char *rootdev, *slash, *dot;

extern SUPER  *sp;
extern GD     *gp;
extern INODE  *ip;
extern DIR    *dp;

extern MINODE minode[NMINODES];
extern MOUNT  mounttab[NMOUNT];
extern OFT    oft[NOFT];

extern char parent[64], child[64], saveParent[64], temp[64],temp2[64];
extern char cwdname[64];

extern char buf[1024], sbuf[1024];
extern char buf12[1024], dblk;
extern char rwbuf[1024];


// buffer.c file

extern struct buf buffer[NBUF], *freelist;
extern int requests, hits;
extern struct devtab devtab[NDEV];
extern struct semaphore freebuf;
extern struct buf *bread();
extern MINODE *iget();

// kernel.c

extern PROC *running;
#define HD     3

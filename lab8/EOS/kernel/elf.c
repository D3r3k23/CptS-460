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

#include "../type.h"

/** loader.c: load user mode image file in ELF format *********

1. need ELF header, Program header types
2. use ld script during linking
3. Example of ld script:
/* A simple ld script */

/* for flat binary ******/
/*
OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(u_entry)
SECTIONS
{
  . = 0x0;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}
*/

/******** for ELF *******/
 /***********************************************************
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(u_entry)
SECTIONS
{
  . = 0x0;
  .text : { *(.text) }
  . = 0x8000;
  .data : { *(.data) }
  . = 0xC000;
  .bss  : { *(.bss)  }
}

4. In the ELF file: text begins at VA=0, data at 0x8000, bss at 0xC000.
   The sections shall be loaded at

   0             0x8000   0xC000                          4MB
   --------------------------------------------------------
   |text (RO)    |data    |bss |                  ustack  |   
   --------------------------------------------------------

5. Loading algorithm:
   1. read ELF header 
   2. read program header to find out offset, filesize
   3. load filesize bytes from offset to page frames of proc
   4  read 4096 bytes to a buf;
      memcpy to a page frame 
*************************************************************/
/* #include "../include.h" */ // WAS for ELF on x86 pmtx
/* #include "type.h"  // ext2 FS, PROC, etc. */

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

// ELF file header
struct elfhdr {
  u32 magic;  // must equal ELF_MAGIC
  u32 dummy[3];
  u16 type;
  u16 machine;
  u32 version;
  u32 entry;
  u32 phoffset;
  u32 shoffset;
  u32 flags;
  u16 ehsize;
  u16 phentsize;
  u16 phnum;
  u16 shentsize;
  u16 shnum;
  u16 shstrndx;
};

// ELF program section header
struct proghdr {
  u32 type;
  u32 offset;
  u32 vaddr;
  u32 paddr;
  u32 filesize;
  u32 memsize;
  u32 flags;  // use this to set page's R/W
  u32 align;
};
/*****************************************************************
         Currently, all Umode image file sizes are < 4MB, 
 ARM: MMU is set up with 1MB sections; kernel=0-1MB, vid fbuffer at 2-3MB
 disk at 3-5MB. ==> each Umode image has 1MB section at 8MB+(pid-1)*1MB =>
 P1 at 8MB, P2 at 9MB, P3 at 10MB, etc ==> up to 120 process Umode images
******************************************************************/
extern char *disk;  // disk base pointer defined and set in t.c file

int bmap, imap, iblk;
char path[64];
int nn;
char *name[32];  // at most 32 component names
char buf[1024], ebuf[1024], tbuf[1024], dbuf[1024];

int breakup(char *path)
{
  int i;
  char *cp;
  cp = path;
  nn = 0;
  
  while (*cp != 0){
       while (*cp == '/') *cp++ = 0;        
       if (*cp != 0)
           name[nn++] = cp;         
       while (*cp != '/' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  /*
  printf("n = %d : ", nn);
  for (i=0; i<nn; i++)
       printf("  %s  ", name[i]);
  printf("\n");
  */
}

int search(INODE *ip, char *name)
{
   int i; 
   char c, *cp, b2[1024];
   DIR  *dp; 
   for (i=0; i<12; i++){
       if ( ip->i_block[i] ){
          getblock(ip->i_block[i], b2);
          cp = b2;
          dp = (DIR *)b2;

          while (cp < b2 + 1024){
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;   
	      //printf("%s ", dp->name); 
              if ( strcmp(dp->name, name) == 0 ){
		//printf("FOUND %s  ", name); 
                 return(dp->inode);
              }
              dp->name[dp->name_len] = c; // restore that last byte
              cp += dp->rec_len;
              dp = (DIR *)cp;
	}
     }
   }
   return 0;
}

// Instead of getblk(blk, buf) all the time, need to implement 
//   int myread(fd, buf, nbytes) and
//   int lseek(fd, position, 0): from file's beginning, get to position_th byte
//  blk = position / BLKSIZE; offset = position % BLKSIZE;
//  then, myread(fd, buf, nbytes)getblk(blk, buf[ ]);
//        copy

int foffset; // current RW position into file

int melseek(int fd, int position, int how) // fd is dummy, only positon matters
{
  u32 oldoffset = foffset;
  foffset = position;
  //printf("lseek: oldoffset=%x  foffset=%x\n", oldoffset, foffset);
  return oldoffset;
}

u32 blkbuf[256]; INODE *iip;
int meread(int fd, char *buf, int nbytes) // fd is dummy; RW from foffset
{
  u32 i, lbk, blk, offset;

  lbk = foffset / BLKSIZE; offset = foffset % BLKSIZE;
  //printf("meread: fosset=%x blk=%x offset=%x\n", foffset, lbk, offset);
  if (lbk < 12)
    blk = iip->i_block[lbk];
  if (lbk >=12 && lbk < 256){
    getblock(iip->i_block[12], (char *)blkbuf);
    lbk -= 12;
    blk = blkbuf[lbk];
  } 
  // printf("blk=%x offset=%x\n", blk,offset);

  getblock(blk, tbuf);  // get a block into tbuf[ ];
  //for (i=0; i<8; i++)
  //  printf("%c| ", tbuf[i]);
  //printf("tbuf=%s\n", tbuf);

  // copy BLKSIZE-offset bytes from tbuf[] into buf[ ]
  for (i=0; i<BLKSIZE-offset; i++){
    buf[i] = tbuf[offset+i];
  }

  foffset += (BLKSIZE - offset); // advance foffset 

  // if offset > 0 ==> need offset more bytes from next blk
  if (offset != 0){
    printf("offset=%d: need %d bytes from next block\n", offset, offset);
    getblock(blk+1, tbuf);
    for (i=BLKSIZE-offset; i<offset; i++){
      buf[i] = tbuf[i];
    }
    foffset += offset;
  }
}

/*
#define VA(x) ((x) + 0x80000000)
#define PA(x) ((x) - 0x80000000)
*/

char b1[1024];
int loadelf(char *filename, PROC *p)
{ 
  int i, fd, phnum, pn, ELF;
   u32 size, count, total;
   char *addr;
   u32  *pgdir, *pgtable;
   struct proghdr *aph, *ph;
   struct elfhdr  *elf;
   int me;
   /*********** can't use open() yet must traverse from / ******** 
   fd = myopen(filename, 0); // open for READ again
   if (fd < 0){
     printf("loader: loading %s failed\n", filename);
     return -1;
   }
  // ARM: also no page buffers
   // allocate page buffers 
   ebuf = (char *)palloc();
   dbuf = (char *)palloc();
************************************************************/
  SUPER *sp;
  GD    *gp;
  INODE *ip;
  DIR   *dp;
  char  *cp, c;
  char  line[32];
  
  //printf("proc %x loading %s disk at %x\n", p->pid, filename, disk);
  color = CYAN;

  //printf("proc %d loading ", p->pid);

  //  kgetline(line);

  fd = 0;      // dummy fd
  foffset = 0; // initialize file RW position to 0 
  kstrcpy(path,filename);

  //printf("loader : tokenlize pathname\n");
  breakup(path);     // break up filename into nn string pointers
                     // in name[i]
  //kgetline(line);
  //  printf("read SUPER block\n");
  getblock(1, buf);
  sp = (SUPER *)buf;
  // printf("magic=%x nblock=%d ninodes=%d\n", sp->s_magic, sp->s_blocks_count, 
  //     sp->s_inodes_count);  

  //printf("read GROUP DESCRIPTOR 0\n");
  getblock(2, buf);
  gp = (GD *)buf;
  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  //printf("bmap=%d imap=%d iblk=%d\n", bmap, imap, iblk);

  //printf("read INODE table block\n");
  getblock(iblk, buf);
  ip = (INODE *)buf + 1;

  /* serach for system name */
  for (i=0; i<nn; i++){
      me = search(ip, name[i]);
      if (me == 0){
          printf("can't find %s\n", name[i]); 
          return(0);
      }
      me--;
      getblock(iblk+(me/8), b1);      /* read block inode of me */
      ip = (INODE *)b1 + (me % 8);
   }

   iip = ip; // set global iip for meread();

   // meread(fd, ebuf, 256, 1); // read elf header
   getblock(ip->i_block[0], ebuf); // read block 0 of file
   elf = (struct elfhdr *)ebuf;
   //printf("elf magic=%x ", elf->magic);

   aph = (struct proghdr *)((char *)elf + elf->phoffset);
   phnum = elf->phnum;
  
   //pgtable = (u32 *)(VA(p->res->pgdir[0]) & 0xFFFFF000); // PA of page+ 7
   // get VA of PROC's pgdir[0] entry, which is the VA of its Umode image area
   pgtable = (u32 *)(p->res->pgdir[2048] & 0xFFF00000); // get VA
   //printf("phnum=%d pgtable=%x", phnum, pgtable);

   //printf("ELF %s\n", filename);
   total = 0;
   for (i=1, ph=aph; i <= phnum; ph++, i++){
       if (ph->type != 1) 
           break;
       /*
       printf("ph->offset=%x vaddr=%x memsize=%x\n", ph->offset, 
	      ph->vaddr, ph->memsize);
       printf("Sec%d: ", i);
       */
       /* MUST FIX this */
       melseek(fd, (long)ph->offset, 0); // set foffset to ph->offset

       pn = ph->vaddr/0x100000; // convert vaddr to page number
       //printf("page=");
       count = 0; 

       addr = (char *)((u32)p->res->pgdir[pn] & 0xFFF00000);
       //printf("loading addr=%x\n", addr);

       while(count < ph->memsize){
	    //printf("%d", pn);
 /************************************************************
 in pgdir the address is PA; only leading 12 bits
 in Kernel, we must access it by VA, which add 0x80000000
 #define VA(x) ((x) + 0x80000000)
 #define PA(x) ((x) - 0x80000000)
 *************************************************************/
	 // printf("addr=%x ", addr);

       /* must fix this */         
       meread(fd, dbuf, BLKSIZE); // read by 1KB BLKSIZE
       //printf("load: %s\n", dbuf);  
       //copy(addr, buf);
       kmemcpy(addr, dbuf, BLKSIZE);  // 1K size since read by 1KB
             addr  += BLKSIZE;
             count += BLKSIZE;
             pn++;
       }
       total += count;
       //printf("\n");
   }
   //printf("total=%d bytes loaded\n", total);
   return 1;
}


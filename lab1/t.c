/*******************************************************
*                  @t.c file                          *
*******************************************************/

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define TRK 18
#define CYL 36
#define BLK 1024

#include "ext2.h"
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

extern char getc(void);
extern void putc(char c);

void prints(char *s);

u16 search(INODE* ip, char* name);
u16 getblk(u16 blk, char* buf);

u16 NSEC = 2;
char buf1[BLK], buf2[BLK];

char* NL = "\n\r";

void main()
{
    // 1. Write YOUR C code to get the INODE of /boot/mtx
    // if INODE has indirect blocks: get i_block[12] into buf2[]

    // 2. setes(0x1000); // MTX loading segment = 0x1000

    // 3. load 12 DIRECT blocks of INODE into memory

    // 4. load INDIRECT blocks, if any, into memory

    u16 ino;
    u16 i, iblk;
    u32* up;
    INODE* ip;
    GD* gp;

    char* name[2];
    name[0] = "boot";
    name[1] = "mtx";

    prints("Locating /"); prints(name[0]); putc('/'); prints(name[1]); prints(NL);

    // Get root inode

    getblk(2, buf1);
    gp = (GD*)buf1;

    iblk = (u16)gp->bg_inode_table;

    getblk(iblk, buf1);
    ip = (INODE*)buf1 + 1; // Root inode (#2)

    // Get boot/mtx inode
    for (i = 0; i < 2; i++) {
        ino = search(ip, name[i]);
        getblk(iblk + (ino - 1) / 8, buf1);
        ip = (INODE*)buf1 + (ino - 1) % 8;
    }

    // Check for indirect blocks
    if (ip->i_block[12]) {
        getblk((u16)ip->i_block[12], buf2);
    }

    // Load mtx image data blocks
    setes(0x1000);

    // Load direct blocks
    for (i = 0; i < 12; i++) {
        getblk((u16)ip->i_block[i], 0);
        putc('*');
        inces();
    }

    // Load indirect blocks
    if (ip->i_block[12]) {
        up = (u32*)buf2;
        while (*up) {
            getblk((u16)(*up), 0);
            putc('.');
            inces();
            up++;
        }
    }

    prints(NL);
    prints("Boot?");
    while (getc() != '\r');
}

void prints(char *s)
{
    while (*s) {
        putc(*s++);
    }
}

u16 search(INODE* ip, char* name)
{
    // search for name in the data block of INODE;
    // return its inumber if found
    // else error();
    u16 i;
    char c;
    char* cp;
    DIR* dp;

    if ((u16)ip->i_block[0]) {
        getblk((u16)ip->i_block[0], buf2);
        cp = (char*)buf2;
        dp = (DIR*)cp;

        while (cp < buf2 + BLK) {
            c = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            prints(dp->name); putc(' ');

            if (strcmp(name, dp->name) == 0) {
                dp->name[dp->name_len] = c;
                prints(NL);
                return (u16)dp->inode;
            } else {
                dp->name[dp->name_len] = c;
                cp += dp->rec_len;
                dp = (DIR*)cp;
            }
        }
    }
    error();
}

u16 getblk(u16 blk, char* buf)
{
    readfd((2*blk) / CYL, ((2*blk) % CYL) / TRK, ((2*blk) % CYL) % TRK, buf);
    // readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}

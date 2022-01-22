/*******************************************************
*                      t.c file                        *
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
void gets(char *s);

int getblk(u16 blk, char* buf);

GD*    gp;
INODE* ip;
DIR*   dp;

char buf1[BLK], buf2[BLK];
int color = 0x0A;
u8 ino;

int main()
{
    while (getc() != '\r')
    {
        u16  i, iblk;
        char c, temp[64];
        char* cp;

        prints("read blk 2 (GD)\n\r");
        getblk(2, buf1);
        gp = (GD*)buf1;

        // 1. WRITE YOUR CODE to get iblk = bg_inode_table block number
        iblk = (u16)gp->bg_inode_table;
        prints("inode_block="); putc(iblk+'0'); prints("\n\r");

        // 2. WRITE YOUR CODE to get root inode
        prints("read inodes begin block to get root inode\n\r");
        getblk(iblk, buf1);
        ip = (INODE*)buf1 + 1; // Root inode

        // 3. WRITE YOUR CODE to step through the data block of root inode
        prints("read data block of root DIR\n\r");
        for (i = 0; i < 12; i++)
        {
            if ((u16)ip->i_block[i] == 0)
                break;
            else
            {
                getblk((u16)ip->i_block[i], buf2);
                cp = (char*)buf2;
                dp = (DIR*)cp;

                while (cp + dp->rec_len < buf2 + BLK)
                {
                    // 4. print file names in the root directory /
                    strncpy(temp, dp->name, dp->name_len);
                    temp[dp->name_len] = '\0';

                    prints(temp);
                    putc(' ');

                    cp += dp->rec_len;
                    dp = (DIR*)cp;
                }
            }
        }
    }
}

void prints(char *s)
{
    while (*s) {
        putc(*s++);
    }
}

void gets(char *s)
{
    while ((*s = getc()) != '\r') {
        putc(*s++);
    }
    *s = '\0';
}

int getblk(u16 blk, char* buf)
{
    // readfd( (2*blk)/CYL, ( (2*blk)%CYL)/TRK, ((2*blk)%CYL)%TRK, buf);
    readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}

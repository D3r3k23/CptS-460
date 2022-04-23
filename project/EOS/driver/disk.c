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

char *disk;

//struct buf  buffer[4] defined in fs/buffer.c

struct buf readbuf, writebuf, rbuf1;  // 2 dedicated buffers

/************* REPLACED by getblock/putblock in sdc.c ***********
int getblock(u32 blk, char *buf)
{
  int i; 
  char *cp = disk + blk*1024;
  for (i=0; i<1024; i++){
    buf[i] = *cp++;
  }
}

int putblock(u32 blk, char *buf)
{
  int i; 
  char *cp = disk + blk*1024;
  for (i=0; i<1024; i++){
    *cp++ = buf[i];
  }
}
*************************************************************/
int get_block(int dev, int blk, char *buf)
{
   getblock(blk, buf);
}
int put_block(int dev, int blk, char *buf)
{
   putblock(blk, buf);
}

struct buf *getblk(int dev, int blk)
{
  // just need a buffer, so let it be the writebuf
  struct buf *bp = &writebuf;
  bp->dev = dev; bp->blk = blk;
  bp->dirty = 0;
  return bp;
} 

struct buf *bread(int dev, int blk)
{
  // use readbuf to read in a block; return readbuf pointer
  // ignore dev here
  struct buf *bp = &readbuf;
  bp->dev = dev; bp->blk = blk;
  getblock(blk, bp->buf);
  return bp;
}

int bwrite(struct buf *bp)
{ 
  // write out bp->buf, which has bp->blk number
  putblock(bp->blk, bp->buf);
}
struct buf *bread1(int dev, int blk)
{
  // use reafbuf to read in a block; return readbuf pointer
  // ignore dev here
  struct buf *bp = &rbuf1;
  bp->dev = dev; bp->blk = blk;
  getblock(blk, bp->buf);
  return bp;
}

int bwrite1(struct buf *bp)
{ 
  // write out bp->buf, which has bp->blk number
  putblock(bp->blk, bp->buf);
}

int awrite(struct buf *bp)
{
  // no awrite; all writes are immediate
  bwrite(bp);
}

int brelse(struct buf *bp)
{
  // no need to release any buffer
  if (bp->dirty)
      bwrite(bp);
}

int bmap, imap, iblk;

int testFS()
{
  SUPER *sp;
  GD    *gp;
  INODE *ip;
  DIR   *dp;
  char *cp, c;
  char buf[1024], buf1[1024], line[32];
  int bino;
 
  // kgetline(line);
  printf("read SUPER block\n");
  getblock(1, buf);
  sp = (SUPER *)buf;
  printf("magic=%x nblock=%d ninodes=%d\n", sp->s_magic, sp->s_blocks_count, 
         sp->s_inodes_count);  

  printf("read GROUP DESCRIPTOR 0\n");
  getblock(2, buf);
  gp = (GD *)buf;
  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmap=%d imap=%d iblk=%d\n", bmap, imap, iblk);


  printf("read INODE table block\n");
  getblock(iblk, buf);
  ip = (INODE *)buf + 1;
  printf("i_block[0]=%d\n", ip->i_block[0]);
  getblock(ip->i_block[0], buf1);

  cp = buf1;
  dp = (DIR *)buf1;

  printf("show contents of / directory\n");
  while(cp < buf1+1024){
    c = dp->name[dp->name_len];
    dp->name[dp->name_len] = 0;
    printf("%d %d %d %s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
    if (strcmp(dp->name, "bin")==0)
      bino = dp->inode;
    dp->name[dp->name_len] = c;
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  printf("show /bin ino=%d\n", bino);
  getblock(iblk+(bino-1)%8, buf);
  ip = (INODE *)buf + (bino-1)%8;

  getblock(ip->i_block[0], buf1);
  cp = buf1;
  dp = (DIR *)buf1;
  while(cp < buf1+1024){
    c = dp->name[dp->name_len];
    dp->name[dp->name_len] = 0;
    printf("%d %d %d %s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

    //    kgetc();

    dp->name[dp->name_len] = c;
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }

}


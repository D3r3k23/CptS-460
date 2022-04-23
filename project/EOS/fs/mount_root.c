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
#include "global.h"
extern struct buf *bread();
extern struct buf *getblk();

int bmap, imap, iblk;
int boot_dev;
char mtbuf[1024];

int mountroot()
{
  int dev, pno;
  MOUNT *mp;
  SUPER *sp;
  MINODE *ip;
  struct buf *bp;
  char rootdev[16];

  /******************   
  boot_dev = 3;   // set by default
  
  dev = pno = boot_dev;
  printf("mount_root: boot_dev=%d\n", boot_dev);

  strcpy(rootdev, "/dev/hda");
  rootdev[strlen(rootdev)+1] = 0;
  rootdev[strlen(rootdev)] = pno+'0'; // assume pno <= 9
  //printf("dev=%d  rootdev=%s\n", dev, rootdev); 
  ****************/
  dev = 3;
  // get super block of rootdev 
  getblock(1, mtbuf);
  sp =(SUPER *)mtbuf; 
  mp = &mounttab[0];

  /* copy super block info into mounttab[0] */
  mp->ninodes = sp->s_inodes_count;
  mp->nblocks = sp->s_blocks_count;
  mp->dev = dev;        // with dev as index, this is redundant but keep it  
  mp->busy = 1;

  strcpy(mp->name, rootdev);
  strcpy(mp->mount_name, "/");
  
  // read root_dev's GD to record its BMAP, IMAP and IBLOCK in mouttab[0]
  
  //  bp = bread(dev, 2);  // Assume all FS 1KB BLKSIZE, GD is in block# 2
  getblock(2, mtbuf);
  gp = (GD *)mtbuf;
  mp->BMAP   = (u32)gp->bg_block_bitmap;
  mp->IMAP   = (u32)gp->bg_inode_bitmap;
  mp->IBLOCK = (u32)gp->bg_inode_table;

  bmap = mp->BMAP;
  imap = mp->IMAP;
  iblk = mp->IBLOCK;
  printf("bmap=%d  imap=%d  iblock=%d\n", bmap, imap, iblk);
  
  /***** call iget(), which inc the Minode's refCount ****/
  root = iget(dev, 2);          // get root inode

  //printf("rootmode=%x\n", root->INODE.i_mode);
  mp->mounted_inode = root;
  root->mountptr = mp;

  // MUST unlock root->lock
  //printf("mount_root  : root->lock=%d ", root->lock.value);
  V(&root->lock);
  //printf("After unlock: root->lock=%d\n", root->lock.value);
  printf("/dev/hda%d mounted on / OK\n", dev);
 
  return(0);
} 

int ksync()
{ 
  MINODE *mip; INODE *ip;
  int i, ref, count;
  struct buf *bp;

  printf("KERNEL: sync file system ....\n");
  printf("Active minodes;\n");
  for (i=0; i<NMINODES; i++){
      mip = &minode[i];
      ip = &mip->INODE;
      if (mip->refCount){
         printf("[%d%d] ref=%d lock=%d\n",
               mip->dev, mip->ino, mip->refCount, mip->lock.value);
      }
  }

  count = 0;
  for (i=0; i<NMINODES; i++){
      mip = &minode[i];
      ip = &mip->INODE;
      if (mip->refCount > 0 && mip->dirty){
        printf("[%d%d] ref=%d lock=%d\n",
                mip->dev, mip->ino, mip->refCount, mip->lock.value);
	ref = mip->refCount;
        mip->refCount = 1;

         P(&mip->lock);
          iput(mip); /* force out */
        mip->dirty = 0;
        mip->refCount = ref;
        count++;
      }
  }
  printf("%d %s ", count, "minodes +");

  // must flush block buffers also
  count = 0;
  for (i=0; i<NBUF; i++){
    bp = &buffer[i];
    if (bp->valid && bp->dirty){
       bp = getblk(bp->dev, bp->blk);  // must lock the buffer
       awrite(bp);
       bp->dirty = 0;
      count++;
    }
  }
  printf("%d buffers flushed\n", count);

  return 0;
}

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

int krmdir(char *y, int z)
{
 MINODE *ip, *pip;  
 DIR *dp;
 char *cp;
 u32 i, j, r, ino, dev, count, done;
 u32 me, myparent;
 u32 size, remain, non_empty;
 char pathname[32], child[16];
 struct buf *bp;
 char buf[1024];

 get_param(y, pathname);

 /* first, get its ino and inode */
 if (pathname[0] == '/') 
     dev = root->dev;
 else
     dev = running->res->cwd->dev;

 ino = getino(&dev, pathname);

 if (!ino){
   //prints("invalid pathname\n");
    return(-1);
 }

 if (ino == 2){
   //prints("mouted filesys; umount first\n");
    return(-1);
 }
 
 ip = iget(dev, ino);

 /* check for DIR type */
 if ((ip->INODE.i_mode & 0040000) != 0040000){
   //prints("not a direcory\n");
    iput(ip);  return(-1);
 }
 
 /* check ownership */

 if (running->res->uid != 0 && running->res->uid != ip->INODE.i_uid){
    prints("not owner\n"); 
    iput(ip);
    return(-1);
 }

 /****** MUST CHECK FOR MOUNTING AS WELL AS cwd */
 if (ip == running->res->cwd){
    prints("can't rmdir cwd\n"); 
    iput(ip);
    return(-1);
 }

 if (ip->refCount > 1){
    prints("dir busy\n");
    iput(ip);
    return(-1);
 }  

 /* then check whether it's empty */
 // link > 2 OR do a search

 if (ip->INODE.i_links_count > 2){
   prints("DIR not empty\n");
   iput(ip);
   return -1;
 }

 size = ip->INODE.i_size;  
 count = non_empty = 0;
 for (i=0; i<12; i++){
      if (ip->INODE.i_block[i]==0)
	break;
      //get_block(ip->dev, (u32)ip->INODE.i_block[i], buf);
      bp = bread(ip->dev, (u32)ip->INODE.i_block[i]);
      dp = (DIR *)bp->buf;
      cp = bp->buf;

      //while (count < ip->INODE.i_size && cp < &bp->buf[1024]){
      while (count < ip->INODE.i_size && cp < &buf[1024]){
        if (dp->inode)
	  non_empty++;
        if (non_empty > 2) break;

        count += dp->rec_len;
        cp += dp->rec_len;
        dp = (DIR *)cp;
      }

      brelse(bp);

      if (non_empty > 2) break;
 }

 if (non_empty > 2){
     prints("dir not empty\n");
     iput(ip);   
     return(-1);
 }

 /* dir is empty, ok to remove */

 /* get parent's ino and inode */
 myparent = findino(ip, &me);
 pip = iget(ip->dev, myparent);

 strcpy(child, (char *)basename(pathname));

 /* remove child from parent directory */
 rm_child(pip, me, child);


 /* deallocate its block and inode */
 for (i=0; i<12; i++){
   if (ip->INODE.i_block[i]==0)
     continue;
   zdalloc(ip->dev, (u32)ip->INODE.i_block[i]);
 }
 idalloc(ip->dev, (u32)ip->ino);
 iput(ip);

 pip->INODE.i_links_count--;   // dec parent's link count by 1
 pip->dirty = 1;   // parent dirty
 iput(pip);
 return(0);
}
/********************
int rm_child(MINODE *parent, u32 me, char *child)
{
  int i, j, done, del_len; u32 count;
  DIR  *dp, *pp;
  char *cp, c;        char *cq; DIR *lastp;
  MINODE *mip = parent;
  struct buf *bp;
  char buf[1024];

  // remove child name from parent directory 
  done = 0;
  count = 0;

  for (i=0; i<12; i++){   // ASSUME: parent DIR has only 12 direct blocks
      if (parent->INODE.i_block[i]==0)
          break;

      //get_block(parent->dev, (u32)parent->INODE.i_block[i], buf);
      bp = bread(parent->dev, (u32)parent->INODE.i_block[i]);
      pp = dp = (DIR *)bp->buf;
      cp = bp->buf;

      while (count < mip->INODE.i_size && cp < &bp->buf[1024]){
        //***** KCW: when unlink, must go by name, not by ino **********
        c = dp->name[dp->name_len];      // get char at end of name
        dp->name[dp->name_len] = 0;
        
        if (strcmp(dp->name, child)==0){ // found child name
           dp->name[dp->name_len] = c;   // restore that char
	   del_len = dp->rec_len;
           dp->inode = 0;                // clear ino to 0

           if (pp != dp){  // dp has predecessor, absorb rec_len
	     printf("pp=[%d%d%s]  dp=[%d%%s]\n", 
                    pp->inode, pp->rec_len, pp->name, 
		    dp->inode, dp->rec_len, dp->name);
              pp->rec_len += dp->rec_len;
           }
           // if dp has trailing entries, move to left 
           printf("mv LEFT: %d ", &bp->buf[BLKSIZE]-cp);
           memcpy(cp, cp+dp->rec_len, (&bp->buf[1024]-cp));
           bp->dirty = 1;
           brelse(bp);                      // bp is modified; must wirte back 
           return 0;  // OK
	}
        dp->name[dp->name_len] = c;      // restore that char
        pp = dp;                         // pp follows dp
        cp += dp->rec_len;               // advance cp;
        count += dp->rec_len;
        dp = (DIR *)cp;
      }
      brelse(bp);
  }
  return -1;  // BAD
}
*******************/
/****************
int rm_child(MINODE *parent, u32 me, char *child)
{
  int i, j, done, del_len; u32 count;
  DIR  *dp, *pp;
  char *cp, c;        char *cq; DIR *lastp;
  MINODE *mip = parent;
  struct buf *bp;
  char buf[1024];

  // remove child name from parent directory 
  done = 0;
  count = 0;

  for (i=0; i<12; i++){   // ASSUME: parent DIR has only 12 direct blocks
      if (parent->INODE.i_block[i]==0)
          break;

      //get_block(parent->dev, (u32)parent->INODE.i_block[i], buf);
      bp = bread(parent->dev, (u32)parent->INODE.i_block[i]);
      pp = dp = (DIR *)bp->buf;
      cp = bp->buf;

      while (count < mip->INODE.i_size && cp < &bp->buf[1024]){
        //***** KCW: when unlink, must go by name, not by ino **********
        c = dp->name[dp->name_len];      // get char at end of name
        dp->name[dp->name_len] = 0;
        
        if (strcmp(dp->name, child)==0){ // found child name
           dp->name[dp->name_len] = c;   // restore that char
	   del_len = dp->rec_len;
           dp->inode = 0;                // clear ino to 0

           // if last entry in block: absorb rec_len into predecessor
           cq = (char *)dp;
           if (cq + dp->rec_len >= &bp->buf[BLKSIZE]){
    printf("this is LAST entry in block %d %s\n", dp->rec_len, dp->name);
    printf("pp is %d %s\n", pp->rec_len, pp->name);

	      pp->rec_len += dp->rec_len;
          }
          else{
             if (dp->rec_len != BLKSIZE){  // must have trailing entries
	        // step to last entry
                printf("del %s ", dp->name);

                cq = bp->buf; lastp = (DIR *)cq;
                while (cq + lastp->rec_len < &bp->buf[1024]){
	           cq += lastp->rec_len;
                   lastp = (DIR *)cq;
                }
                printf("last entry=%d %s\n", lastp->rec_len, lastp->name);
                lastp->rec_len += del_len;
                printf("last entry=%d %s ", lastp->rec_len, lastp->name);
	        // move rest entries LEFT by del_len, i.e
                printf("memcp len=%d\n", &bp->buf[BLKSIZE]-cp);
                memcpy(cp, cp+dp->rec_len, (&bp->buf[1024]-cp));

                cq = bp->buf; lastp=(DIR *)cq;
                while (cq  < &bp->buf[1024]){
		  printf("[%d %s] ", lastp->rec_len, lastp->name);
	           cq += lastp->rec_len;
                   lastp = (DIR *)cq;
                }

	     }
	  }
          //
          bp->dirty = 1;
          brelse(bp);                      // bp is modified; must wirte back 
          return 0;  // OK
	}
        dp->name[dp->name_len] = c;      // restore that char
        pp = dp;                         // pp follows dp
        cp += dp->rec_len;               // advance cp;
        dp = (DIR *)cp;
      }
      brelse(bp);
  }
  return -1;  // BAD
}
*******/

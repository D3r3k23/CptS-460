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

// -------------- DI_entry.c file --------------------------------------
struct buf *getblk();

// KCW: enter_name: always enter as last entry in a block
int enter_name(MINODE *pinode, u32 inumber, char *name)
{
  u32 i, j, blk;
  u32 ideal, need, remain;
  DIR  *dp; 
  char *cp, c;
  MINODE *mip = pinode;
  struct buf *bp;

  //printf("enter_name by NEW algorithm\n");

  need = 4*((strlen(name)+8+3)/4);
  /* parent dir has upto 12 direct blocks */
  for (i=0; i<12; i++){  // assume no non-empty blocks
     blk = pinode->INODE.i_block[i];
     //printf("INSERT: %d%d ", i, blk);
     if (blk==0){
        blk = pinode->INODE.i_block[i] = zalloc(pinode->dev);
        pinode->INODE.i_size += BLKSIZE;
        printf("allocated new DIR block [%d%d]\n", i, blk);

        bp = getblk(pinode->dev, (u32)pinode->INODE.i_block[i]);
        dp = (DIR *)bp->buf;
        dp->inode = inumber;
        dp->name_len = strlen(name);
        dp->rec_len = BLKSIZE;
        strncpy(dp->name, name, dp->name_len);
        bwrite(bp);     // bp modified, must write back
        pinode->dirty = 1;
	printf("inserted first entry %s%d\n", dp->name, dp->rec_len);
        return 1;
     }
     // block existing: read block into bp->buf 
     bp = bread(pinode->dev, (u32)pinode->INODE.i_block[i]);
     dp = (DIR *)bp->buf;
     cp = bp->buf;
     while (cp + dp->rec_len < &bp->buf[BLKSIZE]){// step to last entry in block
        c = dp->name[dp->name_len];
        dp->name[dp->name_len] = 0;
        //printf("%s ", dp->name);
        dp->name[dp->name_len] = c;

        cp += dp->rec_len;
        dp = (DIR *)cp; 
     }
     // dp points at last entry in block
     ideal = 4*((dp->name_len + 8 + 3)/4);
     remain = dp->rec_len - ideal;
     if (remain >= need){
        dp->rec_len = ideal;
        cp += ideal;
        dp = (DIR *)cp;
        dp->inode = inumber;
        dp->name_len = strlen(name);
        dp->rec_len = remain;
        strncpy(dp->name, name, strlen(name));
	//printf("\nremLen=%d\n", dp->rec_len - need);
        bwrite(bp);            // bp modified, must write back
        return 1;
     }
     brelse(bp);               // bp not modified, just release it         
  }
  return 1;
}

// remove child name from parent directory 
int rm_child(MINODE *parent, u32 me, char *child)
{
  int i, j, del_len; 
  DIR  *dp, *pp;
  char *cp, *cq, c; 
  MINODE *mip = parent;
  struct buf *bp;
  
  //printf("rmchild by NEW algorithm\n");

  for (i=0; i<12; i++){   // ASSUME: parent DIR has only 12 direct blocks
      //printf("rm: %d%d\n", i, parent->INODE.i_block[i]);
      if (parent->INODE.i_block[i]==0)
          break;

      bp = bread(parent->dev, (u32)parent->INODE.i_block[i]);
      //showentries(bp->buf);

      pp = dp = (DIR *)bp->buf;
      cp = bp->buf;
      while (cp < &bp->buf[BLKSIZE]){
        c = dp->name[dp->name_len];      // get char at end of name
        dp->name[dp->name_len] = 0;
        
        if (strcmp(dp->name, child)==0){ // found child name
           dp->name[dp->name_len] = c;   // restore that char
	   del_len = dp->rec_len;

           // if first and only entry in block
           if (pp == dp && dp->rec_len == BLKSIZE){ // only entry in block
               printf("ONLY entry in block: dp rec_len=%d\n", dp->rec_len);
               printf("deallocate %d:blk=%d\n", i,parent->INODE.i_block[i]);
               zdalloc(parent->dev, parent->INODE.i_block[i]);
               parent->INODE.i_size -= BLKSIZE;
               parent->dirty = 1;
               // move trailing blocks forward
               for (j=i+1; j<12; j++)
                   parent->INODE.i_block[j-1] = parent->INODE.i_block[j];
               brelse(bp);   // bp not modified, just release it
               return 1;
	   }
           // if last entry in block: absorb rec_len into predecessor
           cq = (char *)dp;
           if (cq + dp->rec_len >= &bp->buf[BLKSIZE]){
	     //printf("LAST entry in block %d %s\n", dp->rec_len, dp->name);
             //printf("pp is %d %s\n", pp->rec_len, pp->name);
	      pp->rec_len += dp->rec_len;
          }
	  else{  // entry in middle of block
             if (dp->rec_len != BLKSIZE){  // must have trailing entries
	        last_entry(bp->buf, del_len);
                memcpy(cp, cp+dp->rec_len, (&bp->buf[BLKSIZE]-cp));
	     }
	  }
	  bwrite(bp);                    // bp modified; must wirte back
          return 1;                      // OK
	}
        dp->name[dp->name_len] = c;      // restore that char
        pp = dp;                         // pp follows dp
        cp += dp->rec_len;               // advance cp;
        dp = (DIR *)cp;
      }
      brelse(bp);                        // bp not modified, just release it
  }
  return -1;                             // BAD
}

// step to last entry, add del_len to rec_len
int last_entry(char *buf, int del_len)
{
  char *cp = buf;
  DIR  *dp = (DIR *)cp;

  while (cp + dp->rec_len < &buf[BLKSIZE]){
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  //printf("last entry=[%d%d%s] ",  dp->inode, dp->rec_len, dp->name);
  dp->rec_len += del_len;
  //printf("last entry=[%d%d%s]\n", dp->inode, dp->rec_len, dp->name);
  return 1;
} 

int showentries(char *buf)
{
  char *cp = buf;
  DIR *dp = (DIR *)buf;

  while(cp<&buf[BLKSIZE]){
    printf("[%d%d%s] ", dp->inode, dp->rec_len,dp->name);
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  return 1;
} 

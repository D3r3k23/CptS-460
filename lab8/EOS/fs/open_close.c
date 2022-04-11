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

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007


int truncate(MINODE *mip)
{
   int i, j; 
   u32 *up, *dup;
   struct buf *bp, *dbp;

   printf("truncate [%d %d]: ", mip->dev, mip->ino);
   
   for (i=0; i<12; i++){
      if (mip->INODE.i_block[i] == 0)
         break;
      zdalloc(mip->dev, mip->INODE.i_block[i]);
      printf("%d ", mip->INODE.i_block[i]);
      mip->INODE.i_block[i] = 0;
   }

   /* deallocate indirect data blocks */
   if (mip->INODE.i_block[12]){
      get_block(mip->dev, mip->INODE.i_block[12], buf12);
      up = (u32 *)buf12;

      for (i=0; i<256; i++){
          if (up[i] == 0)
	     break;
	  printf("%d ", up[i]);
          zdalloc(mip->dev, up[i]);
      }
      zdalloc(mip->dev, mip->INODE.i_block[12]);
      mip->INODE.i_block[12] = 0;
   }

   // deallocate double-indirect blocks
   if (mip->INODE.i_block[13]){
      get_block(mip->dev, mip->INODE.i_block[13], buf12);

      dup = (u32 *)buf12;
 
      for (i=0; i<256; i++){
	  if (dup[i] == 0)
             break;
	  get_block(mip->dev, dup[i], buf);
 
          up = (u32 *)buf; 

          for (j=0; j<256; j++){
              if (up[j] == 0)
		 break;

	      printf("%d ", up[j]);
              zdalloc(mip->dev, up[j]);
          }
	  printf("%d ", dup[i]);	
          zdalloc(mip->dev, dup[i]);
      }
 
      zdalloc(mip->dev, (u16)mip->INODE.i_block[13]);
      mip->INODE.i_block[13] = 0;
   }
   printf("\n");
   for (i=0; i<15; i++)
     mip->INODE.i_block[i] = 0;
   
   mip->INODE.i_blocks = 0; 
   mip->INODE.i_mtime = mip->INODE.i_atime = 0;
   mip->INODE.i_size = 0;
   mip->dirty = 1;
}


int access(MINODE *mip, int mode)
{   
  int uid, gid, fmode, r;

  uid = running->res->uid;
  gid = running->res->gid;

  mode = mode & 0x00F; // mode = 0,1,2

  if (uid == mip->INODE.i_uid) 
     fmode = (mip->INODE.i_mode & OWNER) >> 6;
  else{
       if (gid == mip->INODE.i_gid)
          fmode = (mip->INODE.i_mode & GROUP) >> 3;
       else
          fmode = mip->INODE.i_mode & OTHER;
  }
  
  r = 0;
  switch(mode){
     case 0:  if (fmode & 4)         /* READ access */
                  r = 1;              break;
     case 1:  if (fmode & 2)         /* WRITE access */        
                  r = 1;              break;

     case 2:  if ((fmode & 2) && (fmode & 4))     /* RW access */
                 r = 1;
              break;
  }    
  return r;
}

/*****************
int truncate(MINODE *mip)
{
   int i, j; 
   u32 *up, *dup;
   struct buf *bp, *dbp;

   for (i=0; i<12; i++){
      if (mip->INODE.i_block[i] == 0)
         break;
      zdalloc(mip->dev, (u16)mip->INODE.i_block[i]);
      mip->INODE.i_block[i] = 0;
   }

   // deallocate indirect data blocks
   if (mip->INODE.i_block[12]){
     //get_block(mip->dev, (u16)mip->INODE.i_block[12], buf12);
      bp = bread(mip->dev, (u16)mip->INODE.i_block[12]);
      up = (u32 *)bp->buf;

      for (i=0; i<256; i++){
          if (up[i] == 0)
	     break;
          zdalloc(mip->dev, (u16)up[i]);
      }
      brelse(bp);

      zdalloc(mip->dev, (u16)mip->INODE.i_block[12]);
      mip->INODE.i_block[12] = 0;
   }

   // deallocate double-indirect blocks
   if (mip->INODE.i_block[13]){
      //get_block(mip->dev, (u16)mip->INODE.i_block[13], buf12);

      dbp = bread(mip->dev, (u16)mip->INODE.i_block[13]);
      dup = (u32 *)dbp->buf;
 
      for (i=0; i<256; i++){
	  if (dup[i] == 0)
             break;
	  //get_block(mip->dev, (u16)buf12[i], buf);
          bp = bread(mip->dev, (u16)dup[i]);
          up = (u32 *)bp->buf; 

          for (j=0; j<256; j++){
              if (up[j] == 0)
		 break;
              zdalloc(mip->dev, (u16)up[j]);
          }
          brelse(bp);  
          zdalloc(mip->dev, (u16)dup[i]);
      }
      brelse(dbp);

      zdalloc(mip->dev, (u16)mip->INODE.i_block[13]);
      mip->INODE.i_block[13] = 0;
   }
   mip->INODE.i_blocks = 0; 
   mip->INODE.i_mtime = mip->INODE.i_atime = 0;
   mip->INODE.i_size = 0;
   mip->dirty = 1;
}
****************/

int kopen(char *y, int flag)
{
  int r;
  char pathname[64];

  //  printf("kopen: y=%x ", y);

  get_param(y, pathname);
  //printf("kopen: %s %d\n", pathname, flag);
  if (pathname[0]==0) return -1;
  /* mode = 0, 1 ,2 ,3 for R, W, RW, Append */
  /****************
  mode = z;

  if ( pathname[0]==0 || (mode<0 || mode>3) ){
    //ps2u("bad filename or mode\n\r");
     return(-1);
  }
  ******************/
  r = myopen(pathname, flag);
  //printf("kopen: pathname=%s flag=%d return %d\n", pathname, flag, r);
  //kgetc();
  return r;
}


int myopen(char *pathname, int flag) 
{
  int i, ino, dev;
  MINODE *mip;
  char filename[64];
  OFT *oftp;

  //printf("myopen: pathname=%s flag=%d\n", pathname, flag);

  if (pathname[0] == '/'){
     dev = root->dev;
     strcpy(filename, pathname);
  }
  else{
     dev = running->res->cwd->dev;
     upwd(running->res->cwd);               /* need cwd name string */

     strcpy(filename, cwdname);  /******************** KCW ************/
     strcat(filename, pathname);
  }

  //printf("myopen:%s flag=%x ", filename, flag);

  ino = getino(&dev, filename);

  //printf("\nino=%d\n",ino);// kgetc();

  if (!ino){
    if (flag==0x0041 || flag==0x0241 || flag==0x441){ // WR|CREAT|APPEND
      //printf("no file, creat %s flag=%x\n", pathname, flag);
      if( ocreat(filename) < 0)
        return -1;
      //printf("creat OK\n");
      ino = getino(&dev, filename);
    }
    else{  // NOT O_CREAT
      return -1;
    }
  }
  
  mip = iget(dev,ino);  

  // check permission by running->res->uid and file's rwx rwx rwx
  // need access(mip, flag) function
  if (running->res->uid != 0){
    if (!access(mip, flag)){
        printf("kopen: psermission denied\n");
        iput(mip);
        return -1;
    }
  }

  // check for incompatible open modes: for non-special files only
  if ( !((mip->INODE.i_mode & 0170000)==0020000) && !((mip->INODE.i_mode & 0170000)==0060000) ){
     for (i=0; i<NOFT; i++){
        if (oft[i].refCount > 0 && (oft[i].inodeptr == mip)){
	  if (flag > 0 || oft[i].mode > 0){ // compatible: must be both READ
	     printf("%s already opened with incompatible mode\n", pathname);
             iput(mip);
             return -1;
           }
        }
     }
  }

  oftp = falloc();
  oftp->mode = flag;
  oftp->refCount = 1;
  oftp->inodeptr = mip;

/***********
flag=0x000 RD
flag=0x001 WR                  0000 0001
flag=0x002 RDWR
flag=0x041 WR|CREAT            0100 0001
flag=0x241 WR|CREAT|TRUNC 0010 0100 0001
flag=0x401 WR|APPEND      0100 0000 0001
flag=0x441 WR|CREAT|APPEND

O_RDONLY	     00
O_WRONLY	     01
O_RDWR		     02
O_CREAT		   0100	
O_TRUNC		  01000	
O_APPEND	  02000
**********/

  switch (flag){
     case O_RDONLY : oftp->offset = 0; // RD
                      break;
     case O_WRONLY : // WR
     case O_WRONLY|O_CREAT : // WR|CREAT 
     case O_WRONLY|O_CREAT|O_TRUNC : // WR|CREAT|TRUNC
                   if (mip->INODE.i_size)
                       truncate(mip);
                   oftp->offset = 0;
                   break;
     case O_RDWR   : oftp->offset = 0;   //RDWR
                     break;
     case O_WRONLY|O_APPEND :  
       //case 0_WRONLY|O_CREAT|O_APPEND :
     case 02002:
     case 02101:
     case 02102:
                   oftp->offset =  mip->INODE.i_size;
                   break;
     default: printf("invalid flag\n", flag);
	      iput(mip);
              return(-1);
  }

  for (i = 0; i<NFD; i++){
      if (running->res->fd[i] == 0)
         break;
  }

  if (i>=NFD){
    //ps2u("no more fd entry\n\r");
     fdalloc(oftp);
     iput(mip);
     return(-1);
  }

  running->res->fd[i] = oftp;
  mip->dirty = 1; 

  iunlock(mip);  // unlock minode 

  return(i);
}     
      
int kclose(int fd)
{
  if (fd < 0 || fd >= NFD){
    //ps2u("invalid file descriptor\n\r");
      return(-1);
  }

  if (running->res->fd[fd] == 0){
    //ps2u("invalid fd\n\r");
     return(-1);
  }
 
  return(myclose(fd));

}


int myclose(int fd)
{
  int mode; 
  OFT *oftp; 
  MINODE *mip;

  // printf("%d in myclose fd=%d", running->pid, fd);

  oftp = running->res->fd[fd];
  mode = oftp->mode;

  if (mode==4 || mode==5){ // KCW:mode was 0,1,2,3,4,5
     return(close_pipe(fd));
  }

  running->res->fd[fd] = 0;
  oftp->refCount--;

  if (oftp->refCount > 0)
     return(0);

  mip = oftp->inodeptr;

  ilock(mip);
  iput(mip);

  fdalloc(oftp);

  return(0);
}

/** lseek to at most begin/end of file **/

u32 klseek(int fd, u32 position, int whence)
{
  int mode, old;
  OFT *oftp; MINODE *ip;

  oftp = running->res->fd[fd];
  ip = oftp->inodeptr;
  mode = oftp->inodeptr->INODE.i_mode;
  if (mode & 0100000 == 0100000 || mode & 0040000 == 0040000){ 
      if (position > ip->INODE.i_size) 
          position = ip->INODE.i_size;
  }
  if (whence==0)
      oftp->offset = position;
  if (whence==1)
    oftp->offset += position;
  if (whence==2)
    oftp->offset = ip->INODE.i_size + position;
  /*
  if (fd==3)
    printli(position);
  */
  return (u32)oftp->offset;
}

u32 kfdsize(int fd)
{
  OFT *oftp; MINODE *mip;
  oftp = running->res->fd[fd];
  mip = oftp->inodeptr;
  return mip->INODE.i_size;
}
       
int kdup(int y, int z)
{
   int i;
   for (i=0; i<NFD; i++){
       if (running->res->fd[i]==0){
           running->res->fd[i] = running->res->fd[y];
           running->res->fd[y]->refCount++;
           if (running->res->fd[y]->mode == READ_PIPE)
	     running->res->fd[y]->pipe_ptr->nreader++;
           if (running->res->fd[y]->mode == WRITE_PIPE)
	     running->res->fd[y]->pipe_ptr->nwriter++;

	   //   printf("%d dup %d refcount=%d\n", 
	   //      running->pid, y, running->res->fd[y]->refCount);
           return(i); 
      }
  }
  return(-1);  
}

int kdup2(int y, int z)
{
   if (running->res->fd[z])
       myclose(z);
   running->res->fd[z] = running->res->fd[y];
   running->res->fd[y]->refCount++;
   return(0); 
}

int kchmod(char *ufile, int mode)
{
   int i, ino, dev, r;
   MINODE *mip;  
   INODE *ip;
   long t;
   char pathname[64];

   get_param(ufile, pathname);
 
   if (pathname[0] == '/')
      dev = root->dev;
   else
      dev = running->res->cwd->dev;
   ino = getino(&dev, pathname);

   if (!ino){
      return(-1);
   }

   mip = iget(dev,ino);
   ip = &mip->INODE;   

   if (running->res->uid != 0 && running->res->uid != ip->i_uid){
     //ps2u("not owner\n\r");
      iput(mip);
      return(-1);
   }

   mode = mode & 0777;          /* retain only 9 bits */
   ip->i_mode &= 00177000;
   ip->i_mode |= mode;
   /*ip->time = time(0L); */
   //set_time(mip);
   mip->dirty = 1;             /* modified */
   
   iput(mip);

   return(0);
}


int kchown(char *ufile, int uid)
{
   int i, ino, dev;
   int gid, r;
   MINODE *mip; INODE *ip;
   char pathname[64];
   
   get_param(ufile, pathname);

   if (pathname[0] == '/')
      dev = root->dev;
   else
      dev = running->res->cwd->dev;
   ino = getino(&dev, pathname);

   if (!ino){
      return(-1);
   }

   mip = iget(dev,ino);
   ip =&mip->INODE;

   if (running->res->uid != 0 && running->res->uid != ip->i_uid){
     //ps2u("not owner\n\r");
      iput(mip);
      return(-1);
   }

   ip->i_uid = uid;
   /*ip->time = time(0L);*/
   //set_time(mip);
   mip->dirty = 1;             /* modified */

   iput(mip);

   return(0);
}


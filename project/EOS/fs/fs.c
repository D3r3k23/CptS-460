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

MINODE *root; //, *cwd;
char pathname[64], parameter[64], *name[16];
char names[16][64];
int  nnames;
char *rootdev = "/dev/fd0", *slash = "/", *dot = ".";

SUPER  *sp;
GD     *gp;
INODE  *ip;
DIR    *dp;

MINODE minode[NMINODES];
MOUNT  mounttab[NMOUNT];
OFT    oft[NOFT];

char parent[64], child[64], saveParent[64], temp[64],temp2[64];
char cwdname[64];

char buf[1024], sbuf[1024];
char dblk;
char rwbuf[1024];

/************************
#include "global.c"
#include "buffer.c"
#include "util.c"
#include "mount_root.c"
#include "alloc_dealloc.c"
#include "mkdir_creat.c"
#include "cd_pwd.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "stat.c"
#include "touch.c"
#include "open_close.c"
#include "read.c"
#include "write.c"
#include "dev.c"
#include "mount.c"
************************/

fs_init()
{

  int i,j;
  MINODE *rootip, *mip;

  for (i=0; i<NMINODES; i++){
      mip = &minode[i];
      mip->refCount = 0;
      mip->lock.value = 1;
      mip->lock.queue = 0;
  }

  for (i=0; i<NMOUNT; i++)
      mounttab[i].busy = 0;

  for (i=0; i<NOFT; i++)
      oft[i].refCount = 0;

  printf("mounting root : ");
  mountroot();
}

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

/***********************************************************************
                      io.c file of MTX
***********************************************************************/
char *ctable = "0123456789ABCDEF";

#define CTRL_C 3

int puts(const char *s)
{
  char line[128];
  strcpy(line, s);
  strcat(line, "\n\r");
  write(1, line, strlen(line));
}

#define printk printf

int printf(char *fmt,...);

#define SIGINT 2

typedef struct ext2_dir_entry_2 {
    u32 inode;       /* Inode number */
    u16 rec_len;     /* Directory entry length */
    u8 name_len;    /* Name length */
    u8 file_type;
    char name[255]; /* File name */
} DIR;

typedef struct stat {
  u16    st_dev;     /* major/minor device number */
  u16    st_ino;     /* i-node number */
  u16    st_mode;    /* file mode, protection bits, etc. */
  u16    st_nlink;   /* # links; TEMPORARY HACK: should be nlink_t*/
  u16    st_uid;     /* uid of the file's owner */
  u16    st_gid;     /* gid; TEMPORARY HACK: should be gid_t */
  u16    st_rdev;
  long   st_size;    /* file size */
  long   st_atime;   /* time of last access */
  long   st_mtime;   // time of last modification
  long   st_ctime;   // time of creation
  long   st_dtime;
  long   st_date;
  long   st_time;
} STAT;

// UNIX <fcntl.h> constants: <asm/fcntl.h> in Linux
#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02
#define O_CREAT   0100 /* not fcntl */
#define O_TRUNC   01000 /* not fcntl */
#define O_APPEND  02000

#define S_ISDIR(m) ((m >>  9) == 040)
#define S_ISREG(m) ((m >> 12) == 010)
#define S_ISLNK(m) ((m >> 12) == 012)

// Owner
#define S_IRWXU 0700 // R/W/X
#define S_IRUSR 0400 // R
#define S_IWUSR 0200 // W
#define S_IXUSR 0100 // X

// Group
#define S_IRWXG 0070 // R/W/X
#define S_IRGRP 0040 // R
#define S_IWGRP 0020 // W
#define S_IXGRP 0010 // X

// Others
#define S_IRWXO 0007 // R/W/X
#define S_IROTH 0004 // R
#define S_IWOTH 0002 // W
#define S_IXOTH 0001 // X

#define EOF -1

#define exit mexit

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define C_ISNUMBER(c) (('0' <= (c)) && ((c) <= '9'))
#define C_ISUPPER(c)  (('A' <= (c)) && ((c) <= 'Z'))
#define C_ISLOWER(c)  (('a' <= (c)) && ((c) <= 'z'))

char to_lower(char c)
{
   if (C_ISUPPER(c)) {
      return c + 32;
   } else {
      return c;
   }
}

char to_upper(char c)
{
   if (C_ISLOWER(c)) {
      return c - 32;
   } else {
      return c;
   }
}

int putc(char c)
{
   write(1, &c, 1);
   if (c=='\n')
     putc('\r');
   return 0;
}


void prints(char *s)
{
   while (*s){
      putc(*s);
      s++;
   }
}

void mputs(char *s)
{
  prints(s);
}


extern int strlen(const char *);
void print2f(char *s)
{
  write(2, s, (int)strlen(s));
}

void rpi(int x)
{
   char c;
   if (x==0) return;
   c = ctable[x%10];
   rpi((int)x/10);
   putc(c);
}

void printi(int x)
{
    if (x==0){
       prints("0");
       return;
    }
    if (x < 0){
       putc('-');
       x = -x;
    }
    rpi((int)x);
}

void rpu(u32 x)
{
   char c;
   if (x==0) return;
   c = ctable[x%10];
   rpi((u32)x/10);
   putc(c);
}

void printu(u32 x)
{
    if (x==0){
       prints("0");
       return;
    }
    rpu((u32)x);
}

void rpx(u32 x)
{
   char c;
   if (x==0) return;
   c = ctable[x%16];
   rpx((u32)x/16);
   putc(c);
}

void printx(u32 x)
{
  prints("0x");
   if (x==0){
      prints("0");
      return;
   }
   rpx((u32)x);
}


void printc(char c)
{
  putc(c);
  c = c&0x7F;
  if (c=='\n')
    putc('\r');
}

int printk(char *fmt,...)
{
  char *cp, *cq;
  int  *ip;

  cq = cp = (char *)fmt;
  ip = (int *)&fmt + 1;

  while (*cp){
    if (*cp != '%'){
       printc(*cp);
       cp++;
       continue;
    }
    cp++;
    switch(*cp){
      case 'd' : printi(*ip); break;
      case 'u' : printu(*ip); break;
      case 'x' : printx(*ip); break;
      case 's' : prints((char *)*ip); break;
      case 'c' : printc((char)*ip);   break;
    }
    cp++; ip++;
  }
}

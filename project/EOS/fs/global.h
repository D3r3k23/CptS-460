#define rwbuf sbuf
#define BLOCK_SIZE 1024
extern PROC   *running;
extern SUPER  *sp;
extern GD     *gp;
extern INODE  *ip;
extern DIR    *dp;

extern MINODE *root; 
extern char pathname[64], parameter[64], *name[16];
extern char names[16][64];

extern int  nnames;
extern int  color;
extern int  hd_dev;

extern MINODE minode[NMINODES];
extern MOUNT  mounttab[NMOUNT];
extern OFT    oft[NOFT];

extern char *bufPtr;
extern char *rootdev, *slash, *dot;

extern MINODE *mialloc();
extern MOUNT  *getmountp();

extern char parent[64], child[64], saveParent[64], temp[64],temp2[64];
extern char cwdname[64];

extern char buf[1024], sbuf[1024];
extern char buf12[1024], dblk;

extern int cp2us();

extern struct devtab devtab[ ];
extern struct devsw  devsw[ ];
extern int get_ubyte();
extern int put_ubyte();
extern int kgets(),ugets(), kputc();

extern struct buf   buffer[ ];


extern MINODE *iget();
extern struct buf *bread();

extern u8   btime[8];
extern char cwdname[ ];

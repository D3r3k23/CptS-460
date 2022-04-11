/************************************************************
  Backup Copy of ../include.h in case it's trashed or missing 
*************************************************************/
#include "type.h"

typedef unsigned int uint;

#define KPG_DIR    0x80300000
#define KCODE_SEL	 0x08
#define KDATA_SEL	 0x10
/********* without the LDT, user segments are in GDT ****************
            0    1   2   3   4   5
          |null|KCS|KDS|TSS|UCS|UDS|
 where 4=> index=100|T=0|PL=11 = 0x23, 5=>101|T=0|PL=11 = 0x2B
********************************************************************/
#define USER_CODE_SEL	 0x23
#define USER_DATA_SEL	 0x2B

#define GDT_ADDR    0x80104000
#define GDT_ENTRIES         6
#define GDT_SIZE    (8*GDT_ENTRIES)

#define IDT_ADDR    0x80104800
#define IDT_SIZE       (256*8)

#define GDT_TSS_INDEX       3
#define GDT_TSS_SEL      0x18

/*** UCS selector=0111=index=0, LDT=1, RPL=11; UDS=1111: index=1,LDT,RPL=11 */
#define UCS              0x23
#define UDS              0x2B
#define USS              0x2B
#define UFLAG          0x3202
#define SYS_CALL         0x80

#define VA(x) ((x) + 0x80000000)
#define PA(x) ((x) - 0x80000000)

#define printf printk
int printk(char *fmt,...);

extern int (*intEntry[ ])();
extern int (*intHandler[ ])();
extern void default_tsr(void);

extern int int80h();
extern int hdinth(), kbinth(), tinth(), kcinth(), fdinth(), s0inth(), s1inth();
extern int printh(), cdinth();

extern u8 btime[8];  // centry, year, month, day, bhr, bmin, bsec, bpad;
extern u32 hr, min, sec;

char *strcpy(), *strcat();
struct buf *bread(), *getblk();

extern  int    color;
#define CYAN   0x0A
#define RED    0x0C
#define PURPLE 0x0D

extern void goUmode();

extern PROC  proc[ ];
extern PROC *running;
extern PROC *readyQueue;
extern PROC *freeList;
extern PROC *tfreeList;
extern PROC *sleepList;

extern u64  *gdt;
extern u32  *kpgdir;
extern u32  *pfreeList;

extern int  kernel_init();
extern void loader();


extern MINODE *root;
extern int   ntasks;
extern int   HD;

extern int   hd_rw();
extern int   putc();

extern struct semaphore loadsem;
extern struct semaphore kbData;

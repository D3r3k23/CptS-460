typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

extern u32 *VIC_BASE;
#define VIC_STATUS     0x00/4
#define VIC_INTENABLE  0x10/4
#define VID_VADDR      0x30/4

extern u32 *SIC_BASE;
#define SIC_ENSET      0x08/4
#define SIC_SOFTINTSET 0x10/4
#define SIC_PICENSET   0x20/4

void timer_handler();

#define BLUE   0
#define GREEN  1
#define RED    2
#define WHITE  3
#define CYAN   4
#define YELLOW 5


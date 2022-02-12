#include "type.h"

#include "keymap2"

extern int color;

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

#define CAPS_LOCK 0x58
#define LSHIFT 0x12
#define RSHIFT 0x59
#define CTRL 0x14

#define EOF 0x4

#define EN_LOG 0

#if EN_LOG
    #define LOG(msg) do { kputs("LOG: "); kputs(msg); kputs("\n"); } while (0)
#else
    #define LOG(msg)
#endif

typedef volatile struct kbd
{
    char* base;
    char buf[128];
    int head, tail, data, room;
} KBD;

extern volatile int running;

volatile KBD kbd;

int release;

int caps;
int shift;
int ctrl;

int kbd_init()
{
    kbd.base = (char*)0x10006000;
    *(kbd.base + KCNTL) = 0x10;    // bit4=Enable bit0=INT on
    *(kbd.base + KCLK)  = 8;       // ARM manual says clock=8
    kbd.head = kbd.tail = 0;       // circular buffer char buf[128]
    kbd.data = 0;
    kbd.room = 128;

    release = 0; // Key released interrupt (scode 0xF0)

    caps = 0;  // Caps lock on
    shift = 0; // Shift pressed
    ctrl = 0;  // Ctrl pressed
}

int is_uppercase(void)
{
    if (caps)
        return 1;
    else
        return shift;
}

void kbd_handler()
{
    u8 scode;
    char lowercase_char;
    char uppercase_char;
    char c;
    int press;

    color = YELLOW;

    scode = *(kbd.base + KDATA); // get scan code of this interrpt
    kputs("kbd !interrupt! scancode : "); kprintx(scode);

    if (scode == 0xF0) // Release interrupt
    {
        release = 1;
    }
    else
    {
        if (release == 1) // Key released interrupt
        {
            LOG("Key released");
            release = 0;
            press = 0;
        }
        else // Key pressed interrupt
        {
            LOG("Key pressed");
            press = 1;
        }

        if (scode == LSHIFT || scode == RSHIFT)
        {
            shift = !shift; // Toggle on press or release
            LOG("Shift toggled");
        }
        else if (scode == CTRL)
        {
            ctrl = press;
            if (ctrl)
                LOG("Ctrl set");
            else
                LOG("Ctrl unset");
        }
        else if (press)
        {
            if (scode == CAPS_LOCK)
            {
                caps = !caps;
                LOG("Caps lock toggled");
            }
            else
            {
                // map scode to ASCII
                lowercase_char = ltab[scode];
                uppercase_char = utab[scode];

                if (ctrl)
                {
                    if (lowercase_char == 'c')
                    {
                        kputs("Control-C key\n");
                        running = 0;
                        return;
                    }
                    else if (lowercase_char == 'd')
                    {
                        c = EOF;
                    }
                }
                else
                {
                    c = is_uppercase() ? uppercase_char : lowercase_char;
                }
                kputs("kbd character : '"); kputc(c); kputs("'\n");

                kbd.buf[kbd.head++] = c;
                kbd.head %= 128;
                kbd.data++;
                kbd.room--;
            }
        }
    }
}

int kgetc()
{
    char c;

    unlock();                        // unmask IRQ in case it was masked out
    while (kbd.data == 0);           // BUSY wait while kp->data is 0

    lock();                          // mask out IRQ
    c = kbd.buf[kbd.tail++];
    kbd.tail %= 128;                 /*** Critical Region ***/
    kbd.data--;
    kbd.room++;
    unlock();                        // unmask IRQ
    return c;
}

int kgets(char s[])
{
    char c;
    while ((c = kgetc()) != '\r') {
        *s = c;
        s++;
    }
    *s = '\0';
}


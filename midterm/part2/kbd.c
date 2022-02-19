#include "kbd.h"

#include "vid.h"
#include "string.h"

#include "keymap2"

static volatile KBD kbd;
static int release = 0;

void kbd_init(void)
{
    KBD *kp = &kbd;
    kp->base = (char*)0x10006000;
    *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
    *(kp->base + KCLK)  = 8;
    kp->head = kp->tail = 0;

    kp->data = 0;
    kp->room = 128;

    release = 0;
}

void kbd_handler(void)
{
    u8 scode, c;
    KBD* kp = &kbd;
    //  color = YELLOW;
    scode = *(kp->base + KDATA);

    if (scode == 0xF0) { // key release
        release = 1;
        return;
    }
    if (release) {      // key release code
        release = 0;
        return;
    }

    c = ltab[scode];
    if (c == '\r')
        kputc('\n');
    kputc(c);

    kp->buf[kp->head++] = c;
    kp->head %= 128;
    kp->data++; kp->room--;
}

char kgetc(void)
{
    char c;
    KBD* kp = &kbd;

    while (kp->data == 0);   // BUSY wait for data

    lock();
    c = kp->buf[kp->tail++];
    kp->tail %= 128;
    kp->data--;
    kp->room++;
    unlock();

    return c;
}

int kgets(char* s)
{
    char c;
    KBD *kp = &kbd;

    while ((c = kgetc()) != '\r') {
        if (c == '\b') {
            s--;
            continue;
        }
        *s++ = c;
    }
    *s = 0;
    return strlen(s);
}

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

int svc_handler(volatile int a, int b, int c, int d)
{
    switch (a) {
        case 0  : return kgetpid();
        case 1  : return kgetppid();
        case 2  : return kps();
        case 3  : return kchname((char*)b);
        case 4  : return ktswitch();
        case 5  : return kfork((int)b);
        case 6  : return kwait((int*)b);
        case 7  : return kexit((int)b);
        case 90 : return kgetc() & 0x7F;
        case 91 : return kputc(b);
        case 92 : return kgetPA();
        default:
            printf("invalid syscall %d\n", a);
            return -1;
    }
    // return to goUmode: which will replace r0 in Kstack with r
}


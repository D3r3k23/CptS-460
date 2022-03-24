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

char* PATH = "/bin";

int exec(char* cmdLine) // cmdline=VA in Uspace
{
    // exec.c code as in Chpater 7.7.7: REPLACE loadelf() with load()

    PROC* p = running;

    char kline[128];
    strcpy(kline, cmdLine); // Fetch cmdLine into kernel space

    char* kp = kline;
    while (*kp == ' ') {
        kp++;
    }
    char arg0[32];
    char* cp = arg0;
    while (*kp != ' ' && *kp != '\n' && *kp != '\r' && cp < &arg0[31]) {
        *cp++ = *kp++;
    }
    cp = '\0';

    char filename[32];
    if (!(arg0[0] == '/' || arg0[0] == '.' )) {
        strcpy(filename, PATH);
    }
    strcat(filename, arg0);

    if (!load(filename, p)) {
        printf("Error: Could not load image\n");
        return -1;
    }

    int upa = p->pgdir[2048] & 0xFFFF0000; // PA of Umode image
    int usp = upa + 0x100000 - 128; // cmdLine size < 128

    strcpy((char*)usp, kline);
    p->usp = (int*)VA(0x100000 - 128);

    for (int i = 2; i < 14; i++) {
        p->kstack[SSIZE - i] = 0;
    }
    p->kstack[SSIZE - 1] = (int)VA(0); // Return uLR = VA(0)
    return (int)p->usp; // Replace saved r0 in kstack
}

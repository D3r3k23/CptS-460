#define DIR_BLKSIZE 1024

DIR* read_dir(const char* dirname, char buf[DIR_BLKSIZE], DIR* dir)
{
    if (!dir) {
        STAT st;
        stat(dirname, &st);
        if (S_ISDIR(st.st_mode)) {
            printf("Error: %s is not a dir\n", dirname);
            return NULL;
        } else {
            int fd = open(dirname, O_RDONLY);
            if (fd == NULL) {
                printf("Error: Could not open %s\n", dirname);
                return NULL;
            } else {
                read(fd, buf, DIR_BLKSIZE);
                close(fd);
                return (DIR*)buf;
            }
        }
    } else {
        char* cp = (char*)dir + dir->rec_len;
        if (cp < buf + DIR_BLKSIZE) {
            return (DIR*)cp;
        } else {
            return NULL;
        }
    }
}

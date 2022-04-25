#define DIR_BLKSIZE 1024

DIR* read_dir(const char* dirname, char buf[DIR_BLKSIZE], DIR* dir)
{
    if (!dir) {
        STAT st;
        int r = stat(dirname, &st);
        if (r < 0) {
            return NULL;
        } else {
            int fd = open(dirname, O_RDONLY);
            if (fd == NULL) {
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

void get_dir_entry_name(DIR* dir, char* name)
{
    int len = dir->name_len;
    strncpy(name, dir->name, len);
    return name;
}

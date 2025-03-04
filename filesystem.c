#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

static int my_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {  // Root directory
        stbuf->st_mode = S_IFDIR | 0755;  // Directory with 755 permissions
        stbuf->st_nlink = 2;
    } else {
        return -ENOENT;  // File not found
    }

    return 0;
}

static struct fuse_operations myfs_operations = {
    .getattr = my_getattr,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_operations, NULL);
}

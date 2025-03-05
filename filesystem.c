#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

//getattr function retrieves file or directory metadata
static int my_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        // It's the root directory
        stbuf->st_mode = S_IFDIR | 0755;  // Directory with rwx permissions for owner, and rx for others
        stbuf->st_nlink = 2;  // 2 links: '.' and '..'
    } else if (strcmp(path, "/testfile") == 0) {
        // It's the test file
        stbuf->st_mode = S_IFREG | 0644;  // Regular file with read-write for owner, and read for others
        stbuf->st_nlink = 1;  // 1 link for the file itself
        stbuf->st_size = 0;  // File size is 0 as no content is written yet
    } else {
        return -ENOENT;  // File or directory not found
    }

    return 0;  // Success
}

//readdir function is responsible for listing the contents of a directory
static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    if (strcmp(path, "/") != 0) {
        return -ENOENT;  // Only allow reading the root directory
    }

    // Add "." and ".." entries for the root directory
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // Add a test file entry "testfile"
    filler(buf, "testfile", NULL, 0);

    return 0;  // Successfully return directory content
}




static struct fuse_operations myfs_operations = {
    .getattr = my_getattr,
    .readdir = my_readdir,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_operations, NULL);
}

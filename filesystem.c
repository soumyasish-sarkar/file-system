#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>

static struct fuse_operations myfs_operations = {
    //Function will be added here
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_operations, NULL);
}

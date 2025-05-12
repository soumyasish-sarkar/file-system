// SPDX-License-Identifier: GPL
// Author: Soumyasish Sarkar
// Description: A simple in-kernel custom file system with hard link and file creation support

// ===============================
//           Headers
// ===============================
#include <linux/module.h>       // Core header for loading LKMs into the kernel
#include <linux/fs.h>           // File system structures
#include <linux/init.h>         // Macros used to mark up functions e.g., __init __exit
#include <linux/kernel.h>       // Contains types, macros, functions for the kernel
#include <linux/pagemap.h>      // For simple_dir_inode_operations, etc.
#include <linux/string.h>       // String helper functions
#include <linux/slab.h>         // Kernel memory allocation
#include <linux/uaccess.h>      // For copy_to_user
#include <linux/version.h>
#include <linux/dcache.h> // For d_add
#include <linux/types.h>    // For basic data types like uid_t, gid_t, etc.
#include <linux/mount.h>    // For mounting related functions (if needed)


// ===============================
//        Macro Definitions
// ===============================
#define FILESYSTEM_NAME "file_system"
#define FILESYSTEM_MAGIC 0x1CEB00DA
#define FILESYSTEM_DEFAULT_MODE 0755
#define FILE_CONTENT "Content of myfile\n"
#define FILE_NAME "myfile"
#define MAX_FILE_SIZE 8192 //for file_write()
static char file_data[MAX_FILE_SIZE];
static size_t file_size = 0;


// ===============================
//     Function Declarations
// ===============================
static int file_open(struct inode *inode, struct file *filp);
static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *offset);
static ssize_t file_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset);
static struct inode *file_system_make_inode(struct super_block *sb, int mode);
static struct dentry *file_system_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);
static int file_symlink(struct mnt_idmap *idmap, struct inode *dir,struct dentry *dentry, const char *symname);
static struct dentry *file_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);

// ===============================
//        File Operations
// ===============================
static const struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = file_open,
    .read = file_read,
    .write = file_write,
};

// ===============================
//       Inode Operations
// ===============================
static const struct inode_operations file_system_inode_ops = {
    .link = simple_link,
};

// ===============================
// Directory Inode Operations
// ===============================
static int file_mkdir(struct mnt_idmap *idmap, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    struct inode *inode = file_system_make_inode(dir->i_sb, S_IFDIR | mode);
    if (!inode)
        return -ENOMEM;

    d_add(dentry, inode);

    inode_inc_link_count(dir);      // ".." in new dir points to parent
    inode_inc_link_count(inode);    // "." in new dir points to itself

    printk(KERN_INFO "file_system: Directory '%s' created\n", dentry->d_name.name);
    return 0;
}



static const struct inode_operations file_system_dir_inode_ops = {
    .lookup = file_lookup,
    .link   = simple_link,
    .mkdir  = file_mkdir,
    .rmdir  = simple_rmdir,
    .symlink = file_symlink,
};

// ===============================
//      Softlink
// ===============================
static struct dentry *file_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
    d_add(dentry, NULL);  // Mark it as a negative dentry
    return NULL;
}


static int file_symlink(struct mnt_idmap *idmap, struct inode *dir,
                        struct dentry *dentry, const char *symname)
{
    struct inode *inode;

    inode = new_inode(dir->i_sb);
    if (!inode)
        return -ENOMEM;

    inode->i_mode = S_IFLNK | 0777;
    inode->i_uid = current_fsuid();
    inode->i_gid = current_fsgid();
    inode->i_op = &simple_symlink_inode_operations;

    inode->i_link = kstrdup(symname, GFP_KERNEL);  // Allocate link string
    if (!inode->i_link) {
        iput(inode);
        return -ENOMEM;
    }

    inode->i_size = strlen(symname);  // This is crucial for softlink to work

    //d_add(dentry, inode);  // Add dentry to inode
    d_instantiate(dentry, inode);


    inode_inc_link_count(inode); // Add this to prevent premature inode deletion

    printk(KERN_INFO "file_system: Created symbolic link '%s' -> '%s'\n",
           dentry->d_name.name, symname);

    return 0;
}


// ===============================
//      Superblock Operations
// ===============================
static const struct super_operations file_system_super_ops = {
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

// ===============================
//         Core Functions
// ===============================

// Create a new inode
static struct inode *file_system_make_inode(struct super_block *sb, int mode)
{
    struct inode *inode = new_inode(sb);
    if (!inode)
        return NULL;

    inode->i_ino = get_next_ino();
    inode->i_sb = sb;
    inode->i_mode = mode;

    inode->i_uid = current_fsuid(); 
    inode->i_gid = current_fsgid(); 


    if (S_ISDIR(mode)) {
        inode->i_op = &file_system_dir_inode_ops;
        inode->i_fop = &simple_dir_operations;
        inode_inc_link_count(inode);
    } else if (S_ISREG(mode)) {
        inode->i_fop = &file_ops;
        inode_inc_link_count(inode);
    } else if (S_ISLNK(mode)) {
        inode->i_op = &simple_symlink_inode_operations;
        // i_link will be set in your symlink function
    }

    printk(KERN_INFO "file_system: inode created with mode %o\n", mode);
    return inode;
}

//permission check
static int check_permissions(struct inode *inode, int mask) {
    kuid_t uid = current_fsuid();
    kgid_t gid = current_fsgid();

    umode_t mode = inode->i_mode;

    // Owner
    if (uid_eq(uid, inode->i_uid)) {
        if ((mask & MAY_READ) && !(mode & S_IRUSR)) return -EACCES;
        if ((mask & MAY_WRITE) && !(mode & S_IWUSR)) return -EACCES;
        if ((mask & MAY_EXEC) && !(mode & S_IXUSR)) return -EACCES;
        return 0;
    }

    // Group
    if (gid_eq(gid, inode->i_gid)) {
        if ((mask & MAY_READ) && !(mode & S_IRGRP)) return -EACCES;
        if ((mask & MAY_WRITE) && !(mode & S_IWGRP)) return -EACCES;
        if ((mask & MAY_EXEC) && !(mode & S_IXGRP)) return -EACCES;
        return 0;
    }

    // Others
    if ((mask & MAY_READ) && !(mode & S_IROTH)) return -EACCES;
    if ((mask & MAY_WRITE) && !(mode & S_IWOTH)) return -EACCES;
    if ((mask & MAY_EXEC) && !(mode & S_IXOTH)) return -EACCES;

    return 0;
}


// Ensure default content is added when the file is created
static int file_create_default_content(void) {
    if (file_size == 0) {
        // Add the default content if the file is empty
        strncpy(file_data, FILE_CONTENT, MAX_FILE_SIZE);
        file_size = strlen(FILE_CONTENT);
        printk(KERN_INFO "file_system: Default content added to file\n");
    }
    return 0;
}

static int file_open(struct inode *inode, struct file *filp)
{
    int flags = filp->f_flags;
    int mask = 0;

    if (flags & O_RDONLY) mask |= MAY_READ;
    if (flags & O_WRONLY) mask |= MAY_WRITE;
    if (flags & O_RDWR)   mask |= (MAY_READ | MAY_WRITE);

    if (check_permissions(inode, mask) != 0) {
        printk(KERN_WARNING "file_system: Access denied for open\n");
        return -EACCES;
    }

    file_create_default_content();  // Ensure default content is added to the file when opened
    printk(KERN_INFO "file_system: file_open allowed\n");
    return 0;
}


// Read system call
static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *offset)
{
    if (check_permissions(filp->f_inode, MAY_READ) != 0)
        return -EACCES;

    if (*offset >= file_size)
        return 0;

    if (len > file_size - *offset)
        len = file_size - *offset;

    if (copy_to_user(buf, file_data + *offset, len) != 0)
        return -EFAULT;

    *offset += len;
    return len;
}

// Write system call
static ssize_t file_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset)
{
    if (check_permissions(filp->f_inode, MAY_WRITE) != 0)
        return -EACCES;

    if (*offset + len > MAX_FILE_SIZE)
        len = MAX_FILE_SIZE - *offset;

    if (copy_from_user(file_data + *offset, buf, len) != 0)
        return -EFAULT;

    *offset += len;
    if (*offset > file_size)
        file_size = *offset;

    printk(KERN_INFO "file_system: file_write called, wrote %zu bytes\n", len);
    return len;
}

// ===============================
//        Mount Operations
// ===============================
static int file_system_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode, *file_inode;
    struct dentry *root_dentry, *file_dentry;

    sb->s_magic = FILESYSTEM_MAGIC;
    sb->s_op = &file_system_super_ops;

    // Create root inode
    root_inode = file_system_make_inode(sb, S_IFDIR | FILESYSTEM_DEFAULT_MODE);
    if (!root_inode)
        return -ENOMEM;

    root_dentry = d_make_root(root_inode);
    if (!root_dentry) {
        iput(root_inode);
        return -ENOMEM;
    }

    sb->s_root = root_dentry;

    // Create default file
    file_inode = file_system_make_inode(sb, S_IFREG | 0644);
    if (!file_inode)
        return -ENOMEM;

    file_inode->i_fop = &file_ops;

    file_dentry = d_alloc_name(root_dentry, FILE_NAME);
    if (!file_dentry)
        return -ENOMEM;

    d_add(file_dentry, file_inode);

    printk(KERN_INFO "file_system: superblock initialized with '%s'\n", FILE_NAME);
    return 0;
}

// Mount handler
static struct dentry *file_system_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    printk(KERN_INFO "file_system: mounting...\n");
    return mount_nodev(fs_type, flags, data, file_system_fill_super);
}

// Unmount handler
static void file_system_kill_sb(struct super_block *sb)
{
    struct dentry *root = sb->s_root;
    printk(KERN_INFO "file_system: unmounting...\n");

    if (root) {
        shrink_dcache_sb(sb);
        d_drop(root);
    }

    kill_litter_super(sb);  // Original cleanup
}


// ===============================
//     File System Definition
// ===============================
static struct file_system_type file_system_type = {
    .owner = THIS_MODULE,
    .name = FILESYSTEM_NAME,
    .mount = file_system_mount,
    .kill_sb = file_system_kill_sb,
};

// ===============================
//    Module Init / Exit Hooks
// ===============================
static int __init file_system_init(void)
{
    int ret;

    printk(KERN_INFO "file_system: Registering file system\n");
    ret = register_filesystem(&file_system_type);
    if (ret != 0)
        printk(KERN_ERR "file_system: Registration failed\n");

    return ret;
}

static void __exit file_system_exit(void)
{
    int ret;

    printk(KERN_INFO "file_system: Unregistering file system\n");
    ret = unregister_filesystem(&file_system_type);
    if (ret != 0)
        printk(KERN_ERR "file_system: Unregistration failed\n");
}

module_init(file_system_init);
module_exit(file_system_exit);

// ===============================
//         Module Info
// ===============================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Soumyasish Sarkar");
MODULE_DESCRIPTION("A Simple Kernel-Level File System");

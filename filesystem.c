#include <linux/module.h>   // Required for kernel modules
#include <linux/fs.h>       // File system support
#include <linux/init.h>     // For module initialization
#include <linux/kernel.h>   // For printk()
#include <linux/pagemap.h>  // for simple_dir_inode_operations
#include <linux/string.h>
#include <linux/slab.h>     // Provides dynamic memory allocation functions in the Linux kernel space
#include <linux/uaccess.h>  // For copy_to_user

#define FILESYSTEM_NAME "file_system"  // File system name
#define FILESYSTEM_MAGIC 0x1CEB00DA   // Unique magic number for file system
#define FILESYSTEM_DEFAULT_MODE 0755   // Default file permission for files or directories created
#define FILE_CONTENT "Hello from kernel file system!\n"
#define FILE_NAME "myfile"

// Forward declarations
static int file_open(struct inode *inode, struct file *filp);
static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *offset);
static int file_link(struct dentry *old_dentry, struct inode *dir_inode, struct dentry *new_dentry);
static struct inode *file_system_make_inode(struct super_block *sb, int mode);

// Mount function for the custom file system
static struct dentry *file_system_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

// Kill function for unmounting the file system
static void file_system_kill_sb(struct super_block *sb)
{
    printk(KERN_INFO "file_system: unmounting...\n");
    kill_litter_super(sb);  // Cleans up dentry tree and frees superblock
}

// Define the File System Type
static struct file_system_type file_system_type = {
    .owner = THIS_MODULE,       // Module safety (reference count)
    .name = FILESYSTEM_NAME,
    .mount = file_system_mount,      // Sets up superblock and VFS structures, entry point for setting up the in-memory file system structure
    .kill_sb = file_system_kill_sb,  // Cleans up on unmount
};

// Superblock operations
static const struct super_operations file_system_super_ops = {
    .statfs = simple_statfs,      // Return basic info about the file system
    .drop_inode = generic_delete_inode,  // Called when inode is no longer needed and can be deleted safely
};

// Custom inode operations
static const struct inode_operations file_system_inode_ops = {
    .link = file_link,  // Link function for hard link creation
};

// Inode creation function for the file system
static struct inode *file_system_make_inode(struct super_block *sb, int mode)
{
    struct inode *inode = new_inode(sb);

    if (!inode)
        return NULL;

    inode->i_ino = get_next_ino(); // Unique inode number
    inode->i_sb = sb;
    inode->i_op = &file_system_inode_ops; // Use custom inode operations
    inode->i_fop = &simple_dir_operations;  // Default file operations
    inode->i_mode = mode;
    //inode->i_nlink = 1;  // Initialize the link count to 1 for the new file


    // Don't manually set i_nlink, it will be handled by the kernel.
    inode_inc_link_count(inode);  // Increment the link count for the new inode

    
    printk(KERN_INFO "file_system: inode created with mode %o\n", mode);
    return inode;
}



// Custom open function
static int file_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "file_system: file_open called\n");
    return 0;
}

// Custom read function
static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *offset)
{
    const char *data = FILE_CONTENT;
    size_t datalen = strlen(data);

    printk(KERN_INFO "file_system: file_read called\n");

    if (*offset >= datalen)
        return 0;

    if (len > datalen - *offset)
        len = datalen - *offset;

    if (copy_to_user(buf, data + *offset, len) != 0)
        return -EFAULT;

    *offset += len;
    return len;
}

// Link creation function (hard link)
static int file_link(struct dentry *old_dentry, struct inode *dir_inode, struct dentry *new_dentry)
{
    struct inode *old_inode = d_inode(old_dentry);

    // Increment the link count for the inode
    inode_inc_link_count(old_inode);

    // Create new dentry that points to the same inode
    new_dentry->d_inode = old_inode;
    d_add(new_dentry, old_inode);

    printk(KERN_INFO "file_system: created hard link '%s' to '%s'\n", new_dentry->d_name.name, old_dentry->d_name.name);
    return 0;
}

// Define file operations for our file
static const struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = file_open,
    .read = file_read,
    //.link = file_link, // Uncomment if needed in the future
};

// Superblock setup function for mounting
static int file_system_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode, *file_inode;
    struct dentry *root_dentry, *file_dentry;

    sb->s_magic = FILESYSTEM_MAGIC;
    sb->s_op = &file_system_super_ops;

    // Root inode
    root_inode = file_system_make_inode(sb, S_IFDIR | FILESYSTEM_DEFAULT_MODE);
    if (!root_inode)
        return -ENOMEM;

    root_dentry = d_make_root(root_inode);
    if (!root_dentry)
        return -ENOMEM;

    sb->s_root = root_dentry;

    // Create a file inside root: myfile
    file_inode = file_system_make_inode(sb, S_IFREG | 0644);
    if (!file_inode)
        return -ENOMEM;

    file_inode->i_fop = &file_ops;

    file_dentry = d_alloc_name(root_dentry, FILE_NAME);
    if (!file_dentry)
        return -ENOMEM;

    d_add(file_dentry, file_inode);

    printk(KERN_INFO "file_system: superblock initialized with 'myfile'\n");
    return 0;
}

// Mount function of the file system
static struct dentry *file_system_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    printk(KERN_INFO "file_system: mounting...\n");
    return mount_nodev(fs_type, flags, data, file_system_fill_super);
}

// File system registration function
static int __init file_system_init(void)
{
    int ret;
    printk(KERN_INFO "file_system: Registering file_system\n");

    // Register the file system in the kernel
    ret = register_filesystem(&file_system_type);
    if (ret != 0) {
        printk(KERN_INFO "file_system: Failed to register file_system\n");
        return ret;
    }
    printk(KERN_INFO "file_system: file_system successfully registered\n");

    return 0; // Success
}

// File system unregistration function
static void __exit file_system_exit(void)
{
    int ret;
    printk(KERN_INFO "file_system: Unregistering file_system\n");

    // Unregister the file system
    ret = unregister_filesystem(&file_system_type);
    if (ret != 0) {
        printk(KERN_INFO "file_system: Failed to unregister file_system\n");
    }
    printk(KERN_INFO "file_system: file_system successfully unregistered\n");
}

// Register the module entry and exit points
module_init(file_system_init);  // Entry point when module is loaded (using insmod)
module_exit(file_system_exit);  // Exit point when module is removed (using rmmod)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Soumyasish Sarkar");
MODULE_DESCRIPTION("A Simple Kernel-Level File System");
#include <linux/module.h>   // Required for kernel modules
#include <linux/fs.h>       // File system support
#include <linux/init.h>     // For module initialization
#include <linux/kernel.h>   // For printk()
#include <linux/pagemap.h>  // for simple_dir_inode_operations
#include <linux/string.h>
#include <linux/slab.h>   //provides dynamic memory allocation functions in the Linux kernel space like malloc(), free() in user space.


#define FILESYSTEM_NAME "file_system"  // file_system
#define FILESYSTEM_MAGIC 0x1CEB00DA   //unique magic number for file system
#define FILESYSTEM_DEFAULT_MODE 0755   //default file permision for files or dirctories created

// mount function of the custom file system
static struct dentry *file_system_mount(struct file_system_type *fs_type,int flags, const char *dev_name, void *data);
//
static void file_system_kill_sb(struct super_block *sb)
{
    printk(KERN_INFO "file_system: unmounting...\n");
    kill_litter_super(sb);  // cleans up dentry tree and frees superblock
}

//Define the File System Type
static struct file_system_type file_system_type ={
    .owner = THIS_MODULE,       //Module safety (reference count)
    .name = FILESYSTEM_NAME,
    .mount = file_system_mount,      //Sets up superblock and VFS structures, entry point for setting up the in-memory file system structure.
    .kill_sb = file_system_kill_sb,  //Cleans up on unmount
};

//What the superblock should do when certain things happen
static const struct super_operations file_system_super_ops = {
    .statfs = simple_statfs,      //Return the basic info about the file system
    .drop_inode = generic_delete_inode,  //called when inode no longer needed and can be deleted safely
};

//how this custom file system creates files or directories internally
static struct inode *file_system_make_inode(struct super_block *sb, int mode)
{
    struct inode *inode = new_inode(sb);

    if (!inode) return NULL;

    inode->i_ino = get_next_ino(); // unique inode number
    inode->i_sb = sb;
    inode->i_op = &simple_dir_inode_operations;
    inode->i_fop = &simple_dir_operations;
    inode->i_mode = mode;

    printk(KERN_INFO "file_system: inode created with mode %o\n", mode);
    return inode;
}

//

static int file_system_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;
    sb->s_magic = FILESYSTEM_MAGIC;
    sb->s_op = &file_system_super_ops;

    root_inode = file_system_make_inode(sb, S_IFDIR | FILESYSTEM_DEFAULT_MODE);

    if (!root_inode) {
        printk(KERN_ERR "file_system: failed to allocate root inode\n");
        return -ENOMEM;
    }

    struct dentry *root = d_make_root(root_inode);
    if (!root) {
        printk(KERN_ERR "file_system: failed to create root dentry\n");
        return -ENOMEM;
    }

    sb->s_root = root;

    printk(KERN_INFO "file_system: superblock initialized\n");
    return 0;
}

//
static struct dentry *file_system_mount(struct file_system_type *fs_type,int flags, const char *dev_name, void *data)
{
    printk(KERN_INFO "file_system: mounting...\n");
    return mount_nodev(fs_type, flags, data, file_system_fill_super);
}




//File System Registration Function
static int __init file_system_init(void){   //__init a special Macro, used only during initialization
    int ret;
    printk(KERN_INFO "file_system: Registering file_system\n");

    //register file_system in the kernel
    ret = register_filesystem(&file_system_type);
    if(ret !=0) {
        printk(KERN_INFO "file_sytem: Failed to register file_system\n");
        return ret;
    }
    printk(KERN_INFO "file_system: file_system successfully registered\n");

    return 0; //success
}


//File System Unregistration Function
static void __exit file_system_exit(void){
    int ret;
    printk(KERN_INFO "file_system: Unregistering file_system\n");

    //unregister the file_system
    ret = unregister_filesystem(&file_system_type);
    if (ret != 0){
        printk(KERN_INFO "file_system: Failed to unregister file_system\n");
    }
    printk(KERN_INFO "file_system: file_system successfully unregistered \n");
}


// Step 3: Register the Module Entry and Exit Points
module_init(file_system_init);  /*macro tells the kernel When this module is loaded (using insmod), run the function sfs_init().
So this is the entry point of the module.*/

module_exit(file_system_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Soumyasish Sarkar");
MODULE_DESCRIPTION("A Simple Kernel-Level File System");

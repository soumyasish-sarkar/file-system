#include <linux/module.h>   // Required for kernel modules
#include <linux/fs.h>       // File system support
#include <linux/init.h>     // For module initialization
#include <linux/kernel.h>   // For printk()

#define FILESYSTEM_NAME "sfs"  // Simple File System

// Step 1: File System Registration Function
static int __init sfs_init(void) {
    printk(KERN_INFO "SFS: Initializing Simple File System\n");
    return 0;  // Success
}

// Step 2: File System Unregistration Function
static void __exit sfs_exit(void) {
    printk(KERN_INFO "SFS: Unloading Simple File System\n");
}

// Step 3: Register the Module Entry and Exit Points
module_init(sfs_init);
module_exit(sfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Soumyasish Sarkar");
MODULE_DESCRIPTION("A Simple Kernel-Level File System");

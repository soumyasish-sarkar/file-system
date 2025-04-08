#include <linux/module.h>   // Required for kernel modules
#include <linux/fs.h>       // File system support
#include <linux/init.h>     // For module initialization
#include <linux/kernel.h>   // For printk()

#define FILESYSTEM_NAME "file_system"  // file_system


//File System Registration Function
static int __init file_system_init(void){   //__init a special Macro, used only during initialization
    printk(KERN_INFO "file_system: Initialization of File System\n");
    printk(KERN_INFO "Inside file_syetem_init function i.e, Registering Function\n");

    return 0; //success
}


//File System Unregistration Function
static void __exit file_system_exit(void){
    printk(KERN_INFO "file_system : Unloading file system\n");
    printk(KERN_INFO "Inside file_system_exit function i.e, Unregistering Function\n");
}


// Step 3: Register the Module Entry and Exit Points
module_init(file_system_init);  /*macro tells the kernel When this module is loaded (using insmod), run the function sfs_init().
So this is the entry point of the module.*/

module_exit(file_system_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Soumyasish Sarkar");
MODULE_DESCRIPTION("A Simple Kernel-Level File System");

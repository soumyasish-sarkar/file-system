#include <linux/module.h>   // Required for kernel modules
#include <linux/fs.h>       // File system support
#include <linux/init.h>     // For module initialization
#include <linux/kernel.h>   // For printk()

#define FILESYSTEM_NAME "file_system"  // file_system

//Define the File System Type
static struct file_system_type file_system_type ={
    .owner = THIS_MODULE,       //Module safety (reference count)
    .name = FILESYSTEM_NAME,
   // .mount = simple_mount,      //Sets up superblock and VFS structures, entry point for setting up the in-memory file system structure.
   // .kill_sb = simple_kill_sb,  //Cleans up on unmount
};

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

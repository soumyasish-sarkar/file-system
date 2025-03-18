# User Level File System
User level File Sytem Written in C in Linux Environment (Ubuntu)

# FUSE
**From the [Official Repository](https://github.com/libfuse/libfuse)** 

>FUSE (Filesystem in Userspace) is an interface for userspace programs to export a filesystem to the Linux kernel. The FUSE project consists of two components: the fuse kernel module and the libfuse userspace library. libfuse provides the reference implementation for communicating with the FUSE kernel module.  

Basically, FUSE allows us to call our own functions instead of using the default kernel functions when a system call is used. That is, incoming requests from the kernel are passed to the main program using callbacks. Where we can define our own functions to handle them.   


# Installing FUSE
For Ubuntu
```
$ sudo apt update
$ sudo apt-get install libfuse-dev
```
# Error May occur after installing FUSE Library

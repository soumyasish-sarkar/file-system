# Build Your Own User‑Level File System

<div align="center">
  <img src="/content/tulogo.png" alt="Tezpur University Logo" width="200"/>
</div>

**Bachelor of Technology in Computer Science and Engineering**  
**Soumyasish Sarkar (CSB22054)**  
**Under the guidance of Prof. Sanjib Kumar Deka**  
**Spring 2025**  
**Department of Computer Science & Engineering, Tezpur University**

---

## 📘 Abstract

This project explores the design and development of a custom **kernel-level file system** controlled via a **web-based interface**. Unlike FUSE-based systems, it integrates directly into the Linux kernel, allowing low-level control for file/directory operations, linking, journaling, and permission enforcement. A responsive frontend using HTML/JavaScript with Socket.IO—backed by a Node.js server—enables users to interact with the filesystem in real time, bridging OS-level programming and user-friendly interaction.

---

## 📋 Table of Contents

- [Introduction](#introduction)  
  - [Scope](#scope)  
  - [Purpose](#purpose-of-the-project)  
  - [Report Overview](#overview-of-the-report)
- [Problem Statement](#problem-statement)
- [Software Requirements Specification (SRS)](#software-requirements-specification-srs)  
  - [Functional Requirements](#functional-requirements)  
  - [Non‑Functional Requirements](#non-functional-requirements)  
  - [System Requirements](#system-requirements)
- [Data Flow Diagrams (DFD)](#data-flow-diagrams-dfd)  
  - [Level 0 DFD](#level-0-dfd)  
  - [Level 1 DFD](#level-1-dfd)
- [Objectives](#objectives)
- [Feasibility Study](#feasibility-study)  
  - [Technical](#technical-feasibility)  
  - [Economic](#economic-feasibility)  
  - [Operational](#operational-feasibility)
- [Background Study](#background-study)
- [Proposed Idea](#proposed-idea)
- [System Architecture](#system-architecture)  
  - [Kernel Module](#kernel-module)  
  - [User Interface](#user-interface)  
  - [Backend](#backend)  
  - [Interaction Flow](#interaction-flow)
- [Detailed Flowcharts](#detailed-flowcharts)
- [Code Structure](#code-structure)  
  - [Superblock & Inodes](#superblock--inodes)  
  - [File Operations](#file-operations)  
  - [Inode Operations](#inode-operations)  
  - [Journaling](#journaling)
- [Permissions & Access Control](#permissions--access-control)
- [High-Level Features Implemented](#high-level-features-implemented)
- [Limitations](#limitations)
- [Future Work](#future-work)
- [Conclusion](#conclusion)
- [References](#references)
- [Appendix](#appendix)  
  - [GitHub Code Link](#github-code-link)  
  - [Sample Commands](#sample-commands)  
  - [Screenshots](#screenshots)

---

## Introduction

### Scope
Implementation of a minimalist kernel-level file system integrated with a web UI. Supports file/dir creation, read/write, deletion, hard/symbolic links, permissions, journaling, and UI interaction.

### Purpose of the Project
To bridge OS-level file system internals (inodes, journaling, permissions, mounting) with user-level interaction, providing hands-on experience in kernel development beyond FUSE.

### Overview of the Report
Covers background, feasibility, objectives, architecture, flow diagrams, code structure, features, limitations, results, and future directions.

---

## Problem Statement
Traditional Linux file systems are complex or abstracted via FUSE, limiting educational exposure to real filesystem internals. This project builds a simplified kernel-level filesystem that students can control via a web UI to deepen understanding of inodes, permissions, journaling, kernel calls, and modular FS design.

---

## Software Requirements Specification (SRS)

### Functional Requirements
- **FR1**: Create files & directories  
- **FR2**: Read/write support  
- **FR3**: Hard & symbolic linking  
- **FR4**: Permission checking by user/group  
- **FR5**: Journaling for creation/deletion  
- **FR6**: Web UI displays status  
- **FR7**: Real-time kernel log access

### Non‑Functional Requirements
- **NFR1**: File operations respond within 1 sec  
- **NFR2**: Responsive, mobile-friendly UI  
- **NFR3**: Handle concurrent requests  
- **NFR4**: Secure backend against unauthorized commands

### System Requirements
- Linux OS with kernel module support  
- Node.js runtime  
- Modern browser for frontend

---

## Data Flow Diagrams (DFD)

### Level 0 DFD  
![Level 0 DFD](/content/l0.png)

### Level 1 DFD  
![Level 1 DFD](/content/l1.png)

---

## Objectives
- Implement kernel module filesystem (superblock, inodes).
- Support `open/read/write/unlink/mkdir/create/lookup/symlink/link` operations.
- Enforce permissions using custom `check_permissions()`.
- Manage journaling for create/delete events.
- Provide a modular design with clear separation between OS logic and UI/backend.
- Build a web UI (HTML/CSS/JS + Socket.IO) and Node.js backend with periodic log updates.

---

## Feasibility Study

### Technical Feasibility
- Linux kernel supports loadable modules.  
- Standard web and Node.js tech stack.

### Economic Feasibility
- Open-source tools — no licensing fee.  
- Standard Linux environment suffices.

### Operational Feasibility
- Demonstrates core OS concepts clearly.  
- Extensible design that supports future enhancements.

---

## Background Study
- Linux VFS, superblock, inode, dentry, `file_operations`  
- Unix permissions (UID/GID/i_mode)  
- Kernel vs user space  
- Node.js child processes & Socket.IO integration

---

## Proposed Idea
Implement a kernel-space filesystem with a unique magic number. A web UI sends commands via Node.js backend using `socket.emit()`. Backend executes privileged commands (`touch`, `mkdir`, `ln`, `cat`, `rm`). Kernel module handles operations and logs via `printk()`. UI displays results in real time.

---

## System Architecture

### Kernel Module
Handles operations: `open/read/write`, `create/lookup/mkdir/unlink/symlink`, journaling, and permission logic.

### User Interface
- HTML/CSS frontend – form fields, buttons  
- JavaScript + Socket.IO – sends user actions to backend

### Backend
- Node.js server executing commands with `sudo`  
- Emits operation/status/log messages to UI

### Interaction Flow
1. User triggers action on UI  
2. Frontend emits via Socket.IO to backend  
3. Node.js runs privileged command  
4. Kernel module executes it  
5. `printk()` logs event  
6. UI displays result and logs

---

## Detailed Flowcharts
### 6.1 File Operation Flow  
### 6.1 File Operation Flow  

<div align="center">
  <img src="/content/6.1.png" alt="6.1" width="400"/>
</div>

<div align="center">
  <img src="/content/6.2.png" alt="6.2" width="400"/>
</div>

<div align="center">
  <img src="/content/6.3.png" alt="6.3" width="400"/>
</div>

<div align="center">
  <img src="/content/6.4.png" alt="6.4" width="400"/>
</div>

<div align="center">
  <img src="/content/6.5.png" alt="6.5" width="400"/>
</div>

<div align="center">
  <img src="/content/6.6.png" alt="6.6" width="400"/>
</div>

<div align="center">
  <img src="/content/6.7.png" alt="6.7" width="400"/>
</div>

---

## Code Structure

### Superblock & Inodes
- Magic number: `0x1CEB00DA`  
- Registered on mount  
- Inodes dynamically created

### File Operations
- `file_open()`, `file_read()`, `file_write()` operate on static `file_data[]` buffer

### Inode Operations
- `file_create`, `file_symlink`, `file_unlink`, link counting  
- Permissions enforced via `check_permissions()`

### Journaling
- Logs creation (`C`) and delete (`D`) events in circular buffer

---

## Permissions & Access Control
- Checks based on current UID/GID and file mode  
- Implements Unix-style `rwx` permissions  
- Enforced at every file/directory operation

---

## High-Level Features Implemented

| Feature                     | Description                                                    |
|----------------------------|----------------------------------------------------------------|
| File system registration   | Uses VFS and superblock registration                           |
| Superblock & inode         | Created on mount using `file_system_make_inode()`              |
| File operations            | Supports `open()`, `read()`, `write()`                         |
| Directory operations       | Supports `create`, `mkdir`, `unlink`, `symlink`               |
| Hard & symbolic links      | `simple_link()` and `file_symlink()`                          |
| Journaling                 | Uses `journal_entries[]` to log events                        |
| Permissions                | Enforced via custom logic                                    |
| Default content            | Added via `file_open()`                                      |

---

## Limitations
- Single global buffer makes it non‑scalable  
- Journaling is in-memory (non‑persistent)  
- Basic ACL (without extended permissions)  
- No caching or metadata tracking

---

## Future Work
- Persistent journaling (disk-based)  
- Inode-to-content mapping for multiple files  
- UI-based permission editing  
- Benchmarking and stress testing  
- AI/ML-based access-pattern optimization

---

## Conclusion
The project demonstrates a kernel-level Linux file system with a web-controlled UI. It incorporates key OS components like inodes, journaling, linking, and permission enforcement. Its modular architecture paves the way for future enhancements like persistence, advanced ACLs, and intelligent optimizations.

---

## References
1. Maurice J. Bach, *The Design of the UNIX Operating System* (1986)  
2. Robert Love, *Linux Kernel Development* (3rd edition, 2010)  
3. Jonathan Corbet et al., *Linux Device Drivers* (3rd edition, 2005)  
4. Bovet & Cesati, *Understanding the Linux Kernel* (3rd edition, 2005)  
5. Micah E. Scott, “A Simple Filesystem from Scratch in Linux Kernel Space” (2018)  
6. Ruwaid Louis & David Yu, Reinforcement Learning Study (2019)  
7. Linux Kernel Docs: `filesystems.txt`, `inode.c`  
8. LWN.net VFS articles  
9. Socket.IO documentation  
10. Michael Kerrisk, *The Linux Programming Interface*

---

## Appendix

### GitHub Code Link
[https://github.com/soumyasish-sarkar/file-system](https://github.com/soumyasish-sarkar/file-system)

### Sample Commands
```bash
$ make clean
$ make
$ sudo insmod filesystem.ko
$ sudo mkdir -p /mount/fs
$ sudo mount -t file_system none /mount/fs
$ sudo ls -li /mount/fs
$ sudo dmesg | grep "file_system"
$ sudo ln /mount/fs/myfile /mount/fs/hfile
$ sudo ln -s /mount/fs/myfile /mount/fs/sfile
$ sudo ls -li /mount/fs
$ touch /mnt/fs/myfile
$ cat /mnt/fs/myfile
```

### Screenshots

## 📂 Terminal Output Screenshots

### 📄 File Present
![Listing the file present in the mounted directory](/content/17.3.1.png)

### 🔗 Hard Link Creation
![Creating a hard link to the existing file](/content/17.3.2.png)

### 🔗 Symbolic Link Creation
![Creating a symbolic (soft) link](/content/17.3.3.png)

### ✅ Verifying File, Hard Link, and Soft Link
![Displaying content of original file, hard link, and symbolic link using cat](/content/17.3.4.png)

### 📝 File Creation Using `touch`
![Creating a new file using touch](/content/17.3.5.png)

---

## 🖥️ UI Interface Output Screenshots

### 🟢 Node.js Starting
![Node.js starting](/content/16.4.1.png)

### 🏠 Home Page
![Home page of the application](/content/16.4.2.png)

### 🔄 Directory Content Refresh
![Directory content after refresh](/content/16.4.3.png)

### 🆕 Create File Status Message
![Status message after file creation](/content/16.4.4.png)

### 🗂️ Directory Refresh After File Creation
![Directory content after file creation and refresh](/content/16.4.5.png)

### 📁 Directory Creation Status Message
![Status message after directory creation](/content/16.4.6.png)

### 🔁 Directory Content Refresh After Directory Creation
![Directory content after directory creation and refresh](/content/16.4.7.png)

### 🔗 Hard Link Create Status Message
![Status message after hard link creation](/content/16.4.8.png)

### 🔗 Soft Link Create Status Message
![Status message after soft link creation](/content/16.4.9.png)

---

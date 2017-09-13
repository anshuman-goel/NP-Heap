//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:
//		Anshuman Goel			agoel5
//		Bhushan Thakur		bvthakur
//		Zubin Thampi			zsthampi
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "npheap.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>

extern struct miscdevice npheap_dev;

//extern struct mutex *global_lock;

extern struct linklist
{
								unsigned long offset;
								void* kernel_addr;
								struct linklist *next;
								struct mutex lock;
} *head;

int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
								struct linklist *iter;
								iter = head;
								//printk(KERN_ERR "Core Head %p\n", head);
								printk(KERN_ERR "VMA Offset %lu\n", vma->vm_pgoff*PAGE_SIZE);
								if(head != NULL)
								{
																printk(KERN_ERR "i AM IN IF");

																while (iter->next !=NULL)
																{
																								printk(KERN_ERR "Offset %lu, VMA offset %lu\n", iter->offset, vma->vm_pgoff*PAGE_SIZE);
																								if(iter->offset == vma->vm_pgoff*PAGE_SIZE)
																								{
																																printk(KERN_ERR "i AM IN IF while");
																																if(iter->kernel_addr == NULL)
																																{
																																								printk(KERN_ERR "Allocating Memory %d\n", vma->vm_end - vma->vm_start);
																																								//size_t size = 8192;
																																								// Allocating Kernel Memory
																																								void* kernel_memory = kmalloc(8192, GFP_KERNEL);
																																								printk(KERN_ERR "Memory Allocated\n");
																																								//Creating a mapping from Userspace Virtual Memory to Kernel Logical Memory
																																								//remap_pfn_range(vma, virt_to_phys((void*)((unsigned long)kernel_memory)), vma->vm_pgoff, ksize(kernel_memory), vma->vm_page_prot);
																																								// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
																																								if (remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)kernel_memory) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot) < 0)
																																								{
																																																printk(KERN_ERR "remap_pfn_range failed\n");
																																																return -EIO;
																																								}
																																								printk(KERN_ERR "Memory Mapped");
																																								//Copying the contents from user memory space to kernel memory space
																																								if (copy_from_user(kernel_memory, vma->vm_start, vma->vm_end - vma->vm_start) != 0)
																																								{
																																																printk(KERN_ERR "Cannot copy content from user memory to kernel memory space\n");
																																								}
																																								printk(KERN_ERR "Contents copied");
																																								iter->kernel_addr = kernel_memory;
																																								printk(KERN_ERR "Size %d\n", ksize(kernel_memory));
																																								return 0;
																																}
																																printk(KERN_ERR "Find mapping of size in while %lu\n", ksize(iter->kernel_addr));
																																remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)iter->kernel_addr) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
																																printk(KERN_ERR "Size %d\n", ksize(iter->kernel_addr));
																																if (copy_from_user(iter->kernel_addr, vma->vm_start, vma->vm_end - vma->vm_start) != 0)
																																{
																																								printk(KERN_ERR "Cannot copy content from user memory to kernel memory space\n");
																																}
																																printk(KERN_ERR "Contents copied");
																																return 0;
																								}
																								iter = iter->next;
																}
																if(iter->next == NULL)
																{
																								printk(KERN_ERR "Offset %lu, VMA offset %lu\n", iter->offset, vma->vm_pgoff*PAGE_SIZE);
																								if(iter->offset == vma->vm_pgoff*PAGE_SIZE)
																								{
																																printk(KERN_ERR "i AM IN IF while\n");
																																if(iter->kernel_addr == NULL)
																																{
																																								printk(KERN_ERR "Allocating Memory %d\n", vma->vm_end - vma->vm_start);
																																								//size_t size = 8192;
																																								// Allocating Kernel Memory
																																								void* kernel_memory = kmalloc(8192, GFP_KERNEL);
																																								printk(KERN_ERR "Memory Allocated\n");
																																								//Creating a mapping from Userspace Virtual Memory to Kernel Logical Memory
																																								//remap_pfn_range(vma, virt_to_phys((void*)((unsigned long)kernel_memory)), vma->vm_pgoff, ksize(kernel_memory), vma->vm_page_prot);
																																								// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
																																								if (remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)kernel_memory) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot) < 0)
																																								{
																																																printk(KERN_ERR "remap_pfn_range failed\n");
																																																return -EIO;
																																								}
																																								printk(KERN_ERR "Memory Mapped\n");
																																								//Copying the contents from user memory space to kernel memory space
																																								if (copy_from_user(kernel_memory, vma->vm_start, vma->vm_end - vma->vm_start) != 0)
																																								{
																																																printk(KERN_ERR "Cannot copy content from user memory to kernel memory space\n");
																																								}
																																								printk(KERN_ERR "Contents copied\n");
																																								iter->kernel_addr = kernel_memory;
																																								printk(KERN_ERR "Size %d\n", ksize(kernel_memory));
																																								return 0;
																																}
																																printk(KERN_ERR "Find mapping of size %lu\n", ksize(iter->kernel_addr));
																																remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)iter->kernel_addr) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
																																printk(KERN_ERR "Size %d\n", ksize(iter->kernel_addr));
																																if (copy_from_user(iter->kernel_addr, vma->vm_start, vma->vm_end - vma->vm_start) != 0)
																																{
																																								printk(KERN_ERR "Cannot copy content from user memory to kernel memory space\n");
																																}
																																printk(KERN_ERR "Contents copied");
																																return 0;
																								}
																}
								}
								printk(KERN_ERR "Returning without doing anyting");
								return 0;
}

int npheap_init(void)
{
								int ret;
								if ((ret = misc_register(&npheap_dev)))
																printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
								else
								{
																//mutex_init(global_lock);
																printk(KERN_ERR "\"npheap\" misc device installed\n");
								}
								return ret;
}

void npheap_exit(void)
{
								misc_deregister(&npheap_dev);
}

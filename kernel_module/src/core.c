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
//		Anshuman Goel		agoel5
//		Bhushan Thakur		bvthakur
//		Zubin Thampi		zsthampi
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

struct linklist
{
	unsigned long offset;
	void* kernel_addr;
	struct linklist *next;
} *head=NULL;

int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct linklist *iter;
	iter = head;
	printk(KERN_ERR "VMA Offset %lu\n", vma->vm_pgoff);
	//printk(KERN_ERR "Page size%lu\n", getpagesize());
	if(head != NULL)
	{
		while (iter->next != NULL)
		{
			//printk(KERN_ERR "Offset %lu, VMA offset %lu\n", iter->offset, vma->vm_pgoff);
			if(iter->offset == vma->vm_pgoff)
				{
					printk(KERN_ERR "i AM IN IF");
					remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)iter->kernel_addr) >> PAGE_SIZE, ksize(iter->kernel_addr), vma->vm_page_prot);
					return 0;
				}
			iter = iter->next;
		}
	}
	// Allocating Kernel Memory
	void* kernel_memory = kmalloc(vma->vm_end - vma->vm_start, GFP_KERNEL);

	//Creating a mapping from Userspace Virtual Memory to Kernel Logical Memory
	//remap_pfn_range(vma, virt_to_phys((void*)((unsigned long)kernel_memory)), vma->vm_pgoff, ksize(kernel_memory), vma->vm_page_prot);
	// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
	if (remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)kernel_memory) >> PAGE_SIZE, ksize(kernel_memory), vma->vm_page_prot) < 0)
	{
		printk(KERN_ERR "remap_pfn_range failed\n");
		return -EIO;
	}

	//Copying the contents from user memory space to kernel memory space
	if (copy_from_user(kernel_memory, vma->vm_start, vma->vm_end - vma->vm_start) != 0)
	{
		printk(KERN_ERR "Cannot copy content from user memory to kernel memory space\n");
	}

	struct linklist *temp;
	temp = kmalloc(sizeof(struct linklist), GFP_KERNEL);
	temp->offset = vma->vm_pgoff;
	temp->kernel_addr = kernel_memory;
	temp->next = NULL;
	if(head == NULL)
	{
		head = temp;
	}
	else
	{

		iter->next = temp;
	}
	printk(KERN_ERR "Size %d\n", ksize(kernel_memory));
	//printk(KERN_ERR "%p",temp->kernel_addr);
	return 0;
}

int npheap_init(void)
{
    int ret;
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return ret;
}

void npheap_exit(void)
{
    misc_deregister(&npheap_dev);
}

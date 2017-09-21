// Anshuman Goel,agoel5;Bhushan Thakur,bvthakur;Zubin Thampi,zsthampi;
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

// Linked list structure - The actual definition is in ioctl.c
extern struct linklist
{
	unsigned long offset;
	int size;
	void* kernel_addr;
	struct linklist *next;
	struct mutex lock;
} *head;

int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
	// Get the linked list object
	struct linklist *iter;
	iter = head;

	// Iterate through the linked list
	if(head != NULL)
	{
		while (iter->next !=NULL)
		{
			// If the offset value is found in the linked list
			if(iter->offset == vma->vm_pgoff*PAGE_SIZE)
			{
				// If the kernel address is NULL, allocate memory and continue. 
				// Else, avoid allocating memory, and continue
				if(iter->kernel_addr == NULL)
				{
					iter->size = vma->vm_end - vma->vm_start;
					void* kernel_memory = kzalloc(iter->size, GFP_KERNEL);
					

					// Create a mapping from Userspace Virtual Memory to Kernel Logical Memory
					// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
					if (remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)kernel_memory) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot) < 0)
					{
						return -EIO;
					}

					// Copying the contents from user memory space to kernel memory space
					copy_from_user(kernel_memory, vma->vm_start, vma->vm_end - vma->vm_start);
					// set the kernel memory location in the linked list element
					iter->kernel_addr = kernel_memory;
					return 0;
				}

				// Create a mapping from Userspace Virtual Memory to Kernel Logical Memory
				// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
				remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)iter->kernel_addr) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
				// Copying the contents from user memory space to kernel memory space
				copy_from_user(iter->kernel_addr, vma->vm_start, vma->vm_end - vma->vm_start);
				return 0;
			}

			// Get the next element in the linked list
			iter = iter->next;
		}

		// The previous loop only checks till the second last element in the linked list
		// Check in the last element of the linked list
		if(iter->next == NULL)
		{
			// If the offset value is found in the linked list
			if(iter->offset == vma->vm_pgoff*PAGE_SIZE)
			{
				// If the kernel address is NULL, allocate memory and continue. 
				// Else, avoid allocating memory, and continue
				if(iter->kernel_addr == NULL)
				{
					iter->size = vma->vm_end - vma->vm_start;
					void* kernel_memory = kzalloc(iter->size, GFP_KERNEL);

					// Creating a mapping from Userspace Virtual Memory to Kernel Logical Memory
					// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
					if (remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)kernel_memory) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot) < 0)
					{
						return -EIO;
					}
					// Copying the contents from user memory space to kernel memory space
					copy_from_user(kernel_memory, vma->vm_start, vma->vm_end - vma->vm_start);
					iter->kernel_addr = kernel_memory;
					return 0;
				}
				// Create a mapping from Userspace Virtual Memory to Kernel Logical Memory
				// Ref: https://sites.google.com/site/lbathen/research/mmap_driver
				remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)iter->kernel_addr) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
				// Copying the contents from user memory space to kernel memory space
				copy_from_user(iter->kernel_addr, vma->vm_start, vma->vm_end - vma->vm_start);
				return 0;
			}
		}
	}
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


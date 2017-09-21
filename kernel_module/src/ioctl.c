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

struct mutex global_lock;

struct linklist
{
	unsigned long offset;
	int size;
	void* kernel_addr;
	struct linklist *next;
	struct mutex lock;
} *head=NULL;

// If exist, return the data.
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
	// Check if offset exists in linked list
	struct linklist *iter;
	int flag=0;
	iter = head;

	// We have two types of mutex objects 
	// global_lock => Acquire lock to edit the linked list 
	// lock => For each offset in the linked list 

	// If the linked list is empty, create global lock and continue.
	if(head==NULL)
	{
		printk(KERN_ERR "Head %p\n", head);

		//global_lock =(struct mutex *) kmalloc(sizeof(struct mutex), GFP_KERNEL);
		mutex_init(&global_lock);
		printk(KERN_ERR "Acquiring global lock\n");
	}
	// Acquire global lock
	mutex_lock(&global_lock);
	printk(KERN_ERR "Start IF\n");
	printk(KERN_ERR "USER OFFSET %lu\n", user_cmd->offset);
	// Iterate and check for the offset
	if (iter!=NULL)
	{
		printk(KERN_ERR "Start ieration \n");
		while(iter->next!=NULL) //!iter
		{
			if(iter->offset == user_cmd->offset)
			{
				flag = 1;
				break;
			}
			iter = iter->next;
		}
		if(iter->next==NULL && iter->offset == user_cmd->offset)
		{
			flag = 1;
		}
	}
	printk(KERN_ERR "Finished iterating with flag %d\n", flag);

	// create new structure linklist and add it
	if(!flag)
	{
		struct linklist *temp;
		temp = kmalloc(sizeof(struct linklist), GFP_KERNEL);
		temp->offset = user_cmd->offset;
		temp->next = NULL;
		temp->kernel_addr = NULL;
		user_cmd->size = 0;
		mutex_init(&(temp->lock));
		if(head == NULL)
		{
			head = temp;
		}
		else
		{

			iter->next = temp;
		}
		iter = temp;

		printk(KERN_ERR "CReated node\n");
	}
	printk(KERN_ERR "REleasing global lock");
	mutex_unlock(&global_lock);

	// acquire lock on the offset on linklist
	printk(KERN_ERR "Acquiring local lock\n");
	mutex_lock(&(iter->lock));
	user_cmd->op = 0;

	printk(KERN_ERR "Head %p\n", head);
	if(iter->kernel_addr!=NULL)
	{
		// if (copy_to_user(user_cmd->data, iter->kernel_addr, ksize(iter->kernel_addr)) != 0)
		// {
		//  printk(KERN_ERR "Cannot copy content from kernel memory to user memory space\n");
		// }
		user_cmd->data = iter->kernel_addr;
	}
	//user_cmd->data = iter->kernel_addr;
	return 0;
}

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
	struct linklist *iter = head;
	int flag=0;
	if(iter!=NULL)
	{
		while(iter->next!=NULL)
		{
			if(iter->offset == user_cmd->offset)
			{
				mutex_unlock(&(iter->lock));
				user_cmd->op = 1;
				printk(KERN_ERR "Delete ksize start\n");
				if (iter->kernel_addr != NULL)
				{
					user_cmd->size = ksize(iter->kernel_addr);
					printk(KERN_ERR "User cmd size %d\n", user_cmd->size);
				}
				printk(KERN_ERR "Delete ksize end\n");
				flag=1;
				break;
			}
			iter = iter->next;
		}
		if(iter->offset == user_cmd->offset && !flag)
		{
			mutex_unlock(&(iter->lock));
			user_cmd->op = 1;
			printk(KERN_ERR "Delete ksize start\n");
			if (iter->kernel_addr != NULL)
			{
				user_cmd->size = ksize(iter->kernel_addr);
				printk(KERN_ERR "User cmd size %d\n", user_cmd->size);
			}
			printk(KERN_ERR "Delete ksize end\n");
		}
	}
	return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
	struct linklist *iter = head;
	int size;
	while(iter!=NULL)
	{
		if(iter->offset == user_cmd->offset)
		{
			printk(KERN_ERR "The getsize returns: %d \n",iter->size);
			size = iter->size;
		}

		iter = iter->next;
	}

	return size;

}

long npheap_delete(struct npheap_cmd __user *user_cmd)
{
	struct linklist *iter = head;
	int flag = 0;
	if (iter!=NULL)
	{
		while(iter->next!=NULL)
		{
			if(iter->offset == user_cmd->offset)
			{
				// Free the kernel memory and retain the linklist
				printk(KERN_ERR "Free up memory start\n");
				kfree(iter->kernel_addr);
				printk(KERN_ERR "Free up memory end\n");
				iter->kernel_addr=NULL;
				user_cmd->size = 0;
				flag = 1;
				break;
			}
			iter = iter->next;
		}
		if(iter->offset == user_cmd->offset && !flag)
		{
			// Free the kernel memory and retain the linklist
			printk(KERN_ERR "Free up memory start\n");
			kfree(iter->kernel_addr);
			printk(KERN_ERR "Free up memory end\n");
			iter->kernel_addr=NULL;
			user_cmd->size = 0;
			flag = 1;
		}
	}
	return 0;
}

long npheap_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	switch (cmd) {
		case NPHEAP_IOCTL_LOCK:
		return npheap_lock((void __user *) arg);
		case NPHEAP_IOCTL_UNLOCK:
		return npheap_unlock((void __user *) arg);
		case NPHEAP_IOCTL_GETSIZE:
		return npheap_getsize((void __user *) arg);
		case NPHEAP_IOCTL_DELETE:
		return npheap_delete((void __user *) arg);
		default:
		return -ENOTTY;
	}
}

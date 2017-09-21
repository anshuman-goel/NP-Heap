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

// Linked list structure - they store 
// the offset value, 
// size of memory allocation, 
// the kernel address location (could be NULL if not initialized)
// mutex lock object 
struct linklist
{
	unsigned long offset;
	int size;
	void* kernel_addr;
	struct linklist *next;
	struct mutex lock;
} *head=NULL;


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

		mutex_init(&global_lock);
	}

	// Acquire global lock
	mutex_lock(&global_lock);

	// Iterate through the linked list and check for the offset
	if (iter!=NULL)
	{
		while(iter->next!=NULL) 
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

	// If the offset was not found, create a new node
	if(!flag)
	{
		// Create new node and assign memory
		struct linklist *temp;
		temp = kmalloc(sizeof(struct linklist), GFP_KERNEL);
		temp->offset = user_cmd->offset;
		temp->next = NULL;
		temp->kernel_addr = NULL;
		user_cmd->size = 0;

		// Create mutex object for the offset
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

	}

	// Unlock the global lock, once we're done updating the linked list
	mutex_unlock(&global_lock);

	// acquire lock on the offset on linklist
	mutex_lock(&(iter->lock));
	user_cmd->op = 0;

	// Finally, assign the data to user_cmd object
	if(iter->kernel_addr!=NULL)
	{
		user_cmd->data = iter->kernel_addr;
	}
	return 0;
}

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
	// Get the linked list object
	struct linklist *iter = head;
	int flag=0;

	// Iterate through the linked list to find the offset
	if(iter!=NULL)
	{
		while(iter->next!=NULL)
		{
			// If the offset is found, unlock it, and assign the data to user_cmd object
			if(iter->offset == user_cmd->offset)
			{
				mutex_unlock(&(iter->lock));
				user_cmd->op = 1;
				if (iter->kernel_addr != NULL)
				{
					user_cmd->size = ksize(iter->kernel_addr);
				}
				flag=1;
				break;
			}
			iter = iter->next;
		}

		// The last loop only checks till the second last element
		// Check on the last element
		if(iter->offset == user_cmd->offset && !flag)
		{
			mutex_unlock(&(iter->lock));
			user_cmd->op = 1;
			if (iter->kernel_addr != NULL)
			{
				user_cmd->size = ksize(iter->kernel_addr);
			}
		}
	}
	return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
	// The size is already stored in the linked list object during mmap function
	// Iterate through the linked list, find it, and return it
	struct linklist *iter = head;
	int size;
	while(iter!=NULL)
	{
		if(iter->offset == user_cmd->offset)
		{
			size = iter->size;
		}

		iter = iter->next;
	}

	return size;

}

long npheap_delete(struct npheap_cmd __user *user_cmd)
{
	// To delete, just unlink the linked list node, and free the allotted kernel memory space
	// Also so kernel address as NULL in the linked list element

	struct linklist *iter = head;
	int flag = 0;
	if (iter!=NULL)
	{
		while(iter->next!=NULL)
		{
			if(iter->offset == user_cmd->offset)
			{
				// Free the kernel memory and retain the linklist
				kfree(iter->kernel_addr);
				// Set kernel address as NULL
				// This allocates memory again, if the offset is requested again
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
			kfree(iter->kernel_addr);
			// Set kernel address as NULL
			// This allocates memory again, if the offset is requested again
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

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
//   Author:  Hung-Wei Tseng
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
//#include "core.c"

extern struct mutex *global_lock;

extern struct linklist
{
	unsigned long offset;
	void* kernel_addr;
	struct linklist *next;
	struct mutex *lock;
} *head;

// If exist, return the data.
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    // Check if offset doesn't exist in link linklist.
    struct linklist *iter = head;
    int flag=0;
    // Acquire global lock
    mutex_lock(global_lock);
    //Do something
    if (iter!=NULL)
  {
    while(iter->next!=NULL) //!iter
    {
      if(iter->offset == user_cmd->offset)
      {
        flag = 1;
        break;
      }
      iter = iter -> next;
    }
  }
    // create new structure linklist and add it
    if(!flag)
    {
      struct linklist *temp;
    	temp = kmalloc(sizeof(struct linklist), GFP_KERNEL);
    	temp->offset = user_cmd->offset;
    	temp->next = NULL;
			temp->kernel_addr = NULL;
      mutex_init(temp->lock);
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
    mutex_unlock(global_lock);

    // acquire lock on the offset on linklist
    mutex_lock(iter->lock);
    user_cmd->op = 0;
    return user_cmd->data;
}

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
  struct linklist *iter = head;
  while(iter->next!=NULL)
  {
    if(iter->offset == user_cmd->offset)
    {
      mutex_unlock(iter->lock);
      user_cmd->op = 1;
      break;
    }
    iter = iter -> next;
  }
    return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
    return user_cmd->size;
}
long npheap_delete(struct npheap_cmd __user *user_cmd)
{
	struct linklist *iter = head;
	while(iter->next!=NULL)
	{
		if(iter->offset == user_cmd->offset)
		{
			// Free the kernel memory and retain the linklist
			kfree(iter->kernel_addr);
			iter->kernel_addr=NULL;
			break;
		}
		iter = iter -> next;
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

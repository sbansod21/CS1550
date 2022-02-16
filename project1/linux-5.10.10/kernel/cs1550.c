#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cs1550.h>

static DEFINE_RWLOCK(lock);
static LIST_HEAD(sem_list);
int size = 0;

/**
 * Creates a new semaphore. The long integer value is used to
 * initialize the semaphore's value.
 *
 * The initial `value` must be greater than or equal to zero.
 *
 * On success, returns the identifier of the created
 * semaphore, which can be used with up() and down().
 *
 * On failure, returns -EINVAL or -ENOMEM, depending on the
 * failure condition.
 */
SYSCALL_DEFINE1(cs1550_create, long, value)
{	
	int ret =0;
	//chk if value  is < 0
	if(value < 0)
	{
		return -EINVAL;
	}else
	{
		//write lock here 
		write_lock(&lock);

		//creates a new semaphore: the 1550_sem is like a new objeck
		//using kmalloc init a new sem
		
		struct cs1550_sem *sem =NULL;
		sem = (struct cs1550_sem*)kmalloc(sizeof(struct cs1550_sem), GFP_NOWAIT);

		if(sem == NULL)
		{
		 	return -ENOMEM;
		}
		//lock the list
		//we know that we want to lock the WHOLE SEM LIST so we use the RW lock

		//insert into the semaphore list

		INIT_LIST_HEAD(&sem->list);
		

		list_add(&sem->list, &sem_list);
		//if this is the only sem then set the head and tail to.
		INIT_LIST_HEAD(&sem->waiting_task);
		//then initialize the value and id for the new sem

		sem -> value = value;
			size = size+1;

		sem -> sem_id = size;
		//and then init the new one with its new personal lock
		spin_lock_init(&sem -> lock);

		
		//and then unlock it all.
		write_unlock(&lock);
		ret = size;
	}
	
	return ret;
}

/**
 * Performs the down() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This decrements the value of the semaphore, and *may cause* the
 * calling process to sleep (if the semaphore's value goes below 0)
 * until up() is called on the semaphore by another process.
 *
 * Returns 0 when successful, or -EINVAL or -ENOMEM if an error
 * occurred.
 */
SYSCALL_DEFINE1(cs1550_down, long, sem_id)
{
	int ret = 0;

	read_lock(&lock);
	//now im going to be looking for a certain semaphore 
	
	//piazza
	struct cs1550_sem *trysem;
	struct cs1550_sem *sem;
	//now traverse the list to find it
	list_for_each_entry(trysem, &sem_list, list)
	{//this is going to return a whole semaphore
		if(trysem->sem_id == sem_id)
		{
			sem = trysem;
			break;
			
		}
	}

		//use that sems lock to spinlock
		spin_lock(&sem -> lock);
		//decrement since we are going down
		sem-> value -=1;

		//if its negative that means we have to allocate more memory
		if(sem -> value < 0)
		{	
			//Allocate and insert a task entry to the queue of waiting tasks
			struct cs1550_task *newtask = NULL;
			newtask = (struct cs1550_task *)kmalloc(sizeof(struct cs1550_task), GFP_NOWAIT);

			//initialize lists
			INIT_LIST_HEAD(&(newtask->list));
			newtask->task = current; //Is this correct?
			//missing the add tail
			list_add_tail(&newtask->list, &sem->waiting_task);

			// if(newtask == NULL)
			// {
			// 	ret = -ENOMEM;
			// 	return ret;
			// }
			
			ret = 0;			
			set_current_state(TASK_INTERRUPTIBLE); //puts process to sleep

			spin_unlock(&sem -> lock); //unlock what I locked before

			//piazza says to do this because of the issue
			schedule(); //invokes the scheduler
			
		}else
		{
			ret = 0;
			spin_unlock(&sem -> lock); //unlock what I locked before
		}
			
	read_unlock(&lock);
	return ret;
}	

/**
 * Performs the up() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This increments the value of the semaphore, and *may cause* the
 * calling process to wake up a process waiting on the semaphore,
 * if such a process exists in the queue.
 *
 * Returns 0 when successful, or -EINVAL if the semaphore ID is
 * invalid.
 */
SYSCALL_DEFINE1(cs1550_up, long, sem_id)
{
	int ret = 0;
	//piazza
	read_lock(&lock);
			
	struct cs1550_sem *trysem;
	struct cs1550_sem *sem;
	//now traverse the list to find it
	list_for_each_entry(trysem, &sem_list, list)
	{//this is going to return a whole semaphore
		if(trysem->sem_id == sem_id)
		{
			sem = trysem;
			break;
		}
	}
			//use that sems lock to spinlock
			spin_lock(&sem -> lock);
			//increment since we are going up
			sem-> value +=1;

			if(sem -> value <= 0)
			{
				//piazza 
				//declare the cs1550 task: READ MEput the name as the fisrt and last as 3rd param
				struct cs1550_task *found_task = NULL;
				
				found_task= list_first_entry(&sem->waiting_task, struct cs1550_task, list);
		
				//INIT_LIST_HEAD(&(found_task->list));
				struct task_struct *point = found_task -> task;

				//basically finding the first list entry in the task list and deleting it, I THINK
				list_del(&found_task -> list); 

				wake_up_process(point);
		
		
				spin_unlock(&sem -> lock); //unlock what I locked before
				

				kfree(found_task);			
				ret = 0;
				//RETRIEVE the val of cur and then wake up
				//I think this is what I have to do because I the opposite of this in down??
			
			}else
			{
				ret = 0;
				spin_unlock(&sem -> lock); //unlock what I locked before
			}
	read_unlock(&lock);
	return ret;
}

/**
 * Removes an already-created semaphore from the system-wide
 * semaphore list using the identifier obtained from a previous
 * call to cs1550_create().
 *
 * Returns 0 when successful or -EINVAL if the semaphore ID is
 * invalid or the semaphore's process queue is not empty.
 */
SYSCALL_DEFINE1(cs1550_close, long, sem_id)
{
	int ret = 0;
	write_lock(&lock);

	//piazza
	struct cs1550_sem *trysem;
	struct cs1550_sem *sem;
	//now traverse the list to find it
	list_for_each_entry(trysem, &sem_list, list)
	{//this is going to return a whole semaphore
		if(trysem->sem_id == sem_id)
		{
			sem = trysem;
			break;
		}
	}
			spin_lock(&sem -> lock);
						
			if(list_empty(&sem -> waiting_task))
			{
							
				struct list_head *semaphore = NULL;
				semaphore = &sem -> list;
				
				list_del(&sem -> list);

				kfree(semaphore);
				
				ret = 0;

			}else{
				
				ret = -EINVAL;
			}
	
	spin_unlock(&sem -> lock); //unlock what I locked before
	write_unlock(&lock);
	return ret;
}




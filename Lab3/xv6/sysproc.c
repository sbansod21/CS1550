#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//setPriority Syscall
int sys_setpriority(void)
{ //use the argint function to read from the stack
  // In this function, you need to check that the new priority
  // is valid (in the range of [0, 200]), update the process’s priority,
  //  and, if the new priority is larger than the old priority,
  //  call yield to reschedule. You can use the proc pointer to access
  //  the process control block of the current process.
  int curr, prev;
  prev = myproc()->priority;

  if (argint(0, &curr) < 0)
  {
    return -1;
  }

  if (curr >= 0 && curr <= 200)
  {
    myproc()->priority = curr;
  }
  else
  {
    return -1;
  }

  if (curr > prev)
  {
    yield();
  }

  return prev;
}

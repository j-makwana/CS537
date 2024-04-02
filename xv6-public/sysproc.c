#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "mutex.h"

int sys_fork(void)
{
  return fork();
}

int sys_clone(void)
{
  int fn, stack, arg;
  argint(0, &fn);
  argint(1, &stack);
  argint(2, &arg);
  return clone((void (*)(void *))fn, (void *)stack, (void *)arg);
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
  if (n == 0)
  {
    yield();
    return 0;
  }
  acquire(&tickslock);
  ticks0 = ticks;
  myproc()->sleepticks = n;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  myproc()->sleepticks = -1;
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
void sys_macquire(void)
{
  mutex *lk;
  // declare process
  //struct proc *p = myproc();
  if (argptr(0, (void *)&lk, sizeof(*lk)) < 0)
  {
    return;
  }
  acquire(&lk->lk);
  while (lk->locked)
  {
    pte_t *pte = walkpgdir(myproc()->pgdir, &lk->locked, 0);
    if (pte == 0 || !(*pte & PTE_P))
    {
      // Handle error: page not present
      release(&lk->lk);
      return;
    }
    if (lk->holder != 0 && myproc()->nice < lk->holder->nice)
    {
      for(int i=0;i<MAXPROCNAMELEN;i++){
        if(lk->holder->original_priority[i]==0){
          lk->holder->original_priority[i]=lk->holder->nice;
          break;
        }
      }
      // Temporarily elevate the holder's priority
      lk->holder->nice = myproc()->nice;

    }
    
    uint offset = (uint)&lk->locked % PGSIZE;
    uint physical_address = PTE_ADDR(*pte) + offset;

    sleep((void *)physical_address, &lk->lk);
   
  }
  lk->locked = 1;
  lk->holder = myproc();
  release(&lk->lk);
}
void sys_mrelease(void)
{
  mutex *lk;
//  struct proc *p = myproc();
  if (argptr(0, (void *)&lk, sizeof(*lk)) < 0)
  {
    return;
  }
  acquire(&lk->lk);
  lk->locked = 0;
  
  pte_t *pte = walkpgdir(myproc()->pgdir, &lk->locked, 0);
  uint offset = (uint)&lk->locked % PGSIZE;
  uint physical_address = PTE_ADDR(*pte) + offset;
  if (lk->holder == myproc())
  {
    // Restore the original priority of the holder if it was elevated
    if (lk->holder->nice != lk->holder->original_priority[0])
      //restore it to the last full index
      for(int i=MAXPROCNAMELEN-1;i>=0;i--){
        if(lk->holder->original_priority[i]!=0){
          lk->holder->nice=lk->holder->original_priority[i];
          lk->holder->original_priority[i]=0;
          break;
        }
      }
    
    lk->holder = 0; // Clear the mutex holder
  }
  wakeup((void *)physical_address);
  release(&lk->lk);
}

int sys_nice(void)
{
  int inc;
  if (argint(0, &inc) < 0)
  {
    return -1;
  }

  struct proc *p = myproc();
  p->nice += inc;
  if (p->nice < -20)
    p->nice = -20;
  if (p->nice > 19)
    p->nice = 19;

  p->original_priority[0] = p->nice;

  return 0;
}
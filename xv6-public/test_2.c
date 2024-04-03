#include "types.h"
#include "user.h"

mutex m;

void fn1(void* arg) {
  nice(10); // Lowest priority
  printf(1, "Thread 1 (initial priority %d) trying to acquire lock\n", getnice());
  macquire(&m);
  printf(1, "Thread 1 (priority after acquiring lock %d) acquired lock\n", getnice());
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 1 (priority after releasing lock %d) released lock\n", getnice());
  exit();
}

void fn2(void* arg) {
  sleep(10); // Wait for thread 1 to acquire the lock
  nice(-5); // Higher priority
  printf(1, "Thread 2 (priority %d) trying to acquire lock\n", getnice());
  macquire(&m);
  printf(1, "Thread 2 (priority %d) acquired lock\n", getnice());
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 2 (priority %d) released lock\n", getnice());
  exit();
}

void fn3(void* arg) {
  sleep(20); // Wait for thread 2 to start waiting for the lock
  nice(-10); // Highest priority
  printf(1, "Thread 3 (priority %d) trying to acquire lock\n", getnice());
  macquire(&m);
  printf(1, "Thread 3 (priority %d) acquired lock\n", getnice());
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 3 (priority %d) released lock\n", getnice());
  exit();
}

int main() {
  minit(&m);

  char* stack1 = (char*)malloc(4096);
  char* stack2 = (char*)malloc(4096);
  char* stack3 = (char*)malloc(4096);

  clone(fn1, stack1 + 4096, 0);
  clone(fn2, stack2 + 4096, 0);
  clone(fn3, stack3 + 4096, 0);

  // Wait for all threads to finish
  for (int i = 0; i < 3; i++) {
    wait();
  }

  exit();
}

#include "types.h"
#include "user.h"

mutex m;

void fn1(void* arg) {
  nice(10); // Lowest priority
  printf(1, "Thread 1 (lowest priority) trying to acquire lock\n");
  macquire(&m);
  printf(1, "Thread 1 (lowest priority) acquired lock\n");
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 1 (lowest priority) released lock\n");
  exit();
}

void fn2(void* arg) {
  sleep(10); // Wait for thread 1 to acquire the lock
  nice(-5); // Higher priority
  printf(1, "Thread 2 (higher priority) trying to acquire lock\n");
  macquire(&m);
  printf(1, "Thread 2 (higher priority) acquired lock\n");
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 2 (higher priority) released lock\n");
  exit();
}

void fn3(void* arg) {
  sleep(20); // Wait for thread 2 to start waiting for the lock
  nice(-10); // Highest priority
  printf(1, "Thread 3 (highest priority) trying to acquire lock\n");
  macquire(&m);
  printf(1, "Thread 3 (highest priority) acquired lock\n");
  sleep(50); // Hold the lock for some time
  mrelease(&m);
  printf(1, "Thread 3 (highest priority) released lock\n");
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

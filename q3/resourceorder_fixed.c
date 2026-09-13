#include "types.h"
#include "stat.h"
#include "user.h"

int lock1_pipe[2];
int lock2_pipe[2];

void init_locks() {
  pipe(lock1_pipe);
  pipe(lock2_pipe);
  write(lock1_pipe[1], "L", 1);
  write(lock2_pipe[1], "L", 1);
}

void acquire_lock(int pipe_fd[], char *lock_name, char *proc_name) {
  char buf;
  printf(1, "[%s] Requesting %s...\n", proc_name, lock_name);
  read(pipe_fd[0], &buf, 1);
  printf(1, " -> [%s] ACQUIRED %s\n", proc_name, lock_name);
}

void release_lock(int pipe_fd[], char *lock_name, char *proc_name) {
  write(pipe_fd[1], "L", 1);
  printf(1, " -> [%s] RELEASED %s\n", proc_name, lock_name);
}

int main(int argc, char *argv[]) {
  printf(1, "=== DEADLOCK PREVENTION (Strict Resource Ordering) ===\n\n");
  init_locks();

  int pid = fork();

  if (pid == 0) {
    // --- CHILD PROCESS B ---
    // FIXED ORDER: Always Lock1 first, then Lock2
    acquire_lock(lock1_pipe, "Lock1", "Process B");
    sleep(5);
    acquire_lock(lock2_pipe, "Lock2", "Process B");

    // Critical Section
    printf(1, " *** [Process B] Inside Critical Section ***\n");

    release_lock(lock2_pipe, "Lock2", "Process B");
    release_lock(lock1_pipe, "Lock1", "Process B");
    exit();
  } else {
    // --- PARENT PROCESS A ---
    // FIXED ORDER: Always Lock1 first, then Lock2
    acquire_lock(lock1_pipe, "Lock1", "Process A");
    sleep(5);
    acquire_lock(lock2_pipe, "Lock2", "Process A");

    // Critical Section
    printf(1, " *** [Process A] Inside Critical Section ***\n");

    release_lock(lock2_pipe, "Lock2", "Process A");
    release_lock(lock1_pipe, "Lock1", "Process A");

    wait(); // Wait for child to complete
    printf(1, "\nSUCCESS: Both processes completed without deadlocking.\n");
    exit();
  }
}
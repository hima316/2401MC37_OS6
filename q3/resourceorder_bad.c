#include "types.h"
#include "stat.h"
#include "user.h"

// Simulated user-space locks using xv6 pipes as binary semaphores
int lock1_pipe[2];
int lock2_pipe[2];

void init_locks() {
  pipe(lock1_pipe);
  pipe(lock2_pipe);
  // Seed each pipe with 1 byte (represents available lock)
  write(lock1_pipe[1], "L", 1);
  write(lock2_pipe[1], "L", 1);
}

void acquire_lock(int pipe_fd[], char *lock_name, char *proc_name) {
  char buf;
  printf(1, "[%s] Requesting %s...\n", proc_name, lock_name);
  read(pipe_fd[0], &buf, 1); // Blocks if lock is held
  printf(1, " -> [%s] ACQUIRED %s\n", proc_name, lock_name);
}

void release_lock(int pipe_fd[], char *lock_name, char *proc_name) {
  write(pipe_fd[1], "L", 1);
  printf(1, " -> [%s] RELEASED %s\n", proc_name, lock_name);
}

int main(int argc, char *argv[]) {
  printf(1, "=== DEADLOCK DEMONSTRATION (Resource Ordering Violation) ===\n\n");
  init_locks();

  int pid = fork();

  if (pid == 0) {
    // --- CHILD PROCESS B ---
    // Inconsistent Order: Requests Lock2 first, then Lock1
    acquire_lock(lock2_pipe, "Lock2", "Process B");

    // Artificial delay to ensure Process A grabs Lock1
    sleep(10); 

    acquire_lock(lock1_pipe, "Lock1", "Process B");

    // Critical Section
    printf(1, "[Process B] Completed critical section.\n");

    release_lock(lock1_pipe, "Lock1", "Process B");
    release_lock(lock2_pipe, "Lock2", "Process B");
    exit();
  } else {
    // --- PARENT PROCESS A ---
    // Inconsistent Order: Requests Lock1 first, then Lock2
    acquire_lock(lock1_pipe, "Lock1", "Process A");

    // Artificial delay to ensure Process B grabs Lock2
    sleep(10);

    acquire_lock(lock2_pipe, "Lock2", "Process A");

    // Critical Section
    printf(1, "[Process A] Completed critical section.\n");

    release_lock(lock2_pipe, "Lock2", "Process A");
    release_lock(lock1_pipe, "Lock1", "Process A");

    wait(); // Wait for child
    exit();
  }
}
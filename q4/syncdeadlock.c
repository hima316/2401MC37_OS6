#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCESSES 5
#define NUM_RESOURCES 3
#define CYCLES 2

// Resource pool instance counts: 2 Printers (R0), 1 Scanner (R1), 2 Disks (R2)
int resource_instances[NUM_RESOURCES] = {2, 1, 2};

// Pipes used as counting semaphores for each resource pool
int res_pipes[NUM_RESOURCES][2];

// Resource combinations needed by each process (2 resources per process)
int process_needs[NUM_PROCESSES][2] = {
  {0, 1}, // P0 needs Printer (R0) and Scanner (R1)
  {1, 2}, // P1 needs Scanner (R1) and Disk (R2)
  {0, 2}, // P2 needs Printer (R0) and Disk (R2)
  {0, 1}, // P3 needs Printer (R0) and Scanner (R1)
  {1, 2}  // P4 needs Scanner (R1) and Disk (R2)
};

// Initialize resource pools using pipe tokens
void init_semaphores() {
  for (int i = 0; i < NUM_RESOURCES; i++) {
    if (pipe(res_pipes[i]) < 0) {
      printf(1, "Error creating pipe for resource %d\n", i);
      exit();
    }
    // Seed pipe with tokens equal to available instances
    for (int j = 0; j < resource_instances[i]; j++) {
      write(res_pipes[i][1], "T", 1);
    }
  }
}

// Acquire 1 instance of a resource (blocks if count is 0)
void acquire_resource(int res_id, int pid) {
  char token;
  printf(1, "[P%d] Requesting Resource R%d...\n", pid, res_id);
  read(res_pipes[res_id][0], &token, 1);
  printf(1, " -> [P%d] GRANTED Resource R%d\n", pid, res_id);
}

// Release 1 instance of a resource back to pool
void release_resource(int res_id, int pid) {
  write(res_pipes[res_id][1], "T", 1);
  printf(1, " -> [P%d] RELEASED Resource R%d\n", pid, res_id);
}

// Process task simulation loop
void run_process_work(int pid) {
  int req1 = process_needs[pid][0];
  int req2 = process_needs[pid][1];

  // DEADLOCK PREVENTION RULE: Enforce global ascending order (Resource Ordering)
  int first_res = (req1 < req2) ? req1 : req2;
  int second_res = (req1 < req2) ? req2 : req1;

  for (int cycle = 1; cycle <= CYCLES; cycle++) {
    printf(1, "\n--- Process P%d starting Cycle %d/%d ---\n", pid, cycle, CYCLES);

    // Acquire resources strictly in ordered sequence
    acquire_resource(first_res, pid);
    sleep(2); // Short delay between requests to test concurrency
    acquire_resource(second_res, pid);

    // Perform critical work with acquired resources
    printf(1, " *** [P%d] Doing WORK with R%d and R%d (Cycle %d) ***\n", 
           pid, first_res, second_res, cycle);
    sleep(5); // Simulated work

    // Release resources in reverse order
    release_resource(second_res, pid);
    release_resource(first_res, pid);

    printf(1, "[P%d] Completed Cycle %d/%d successfully\n", pid, cycle, CYCLES);
    sleep(2); // Rest before next iteration
  }
}

int main(int argc, char *argv[]) {
  printf(1, "=== MULTI-RESOURCE SYNCHRONIZATION & DEADLOCK AVOIDANCE ===\n");
  printf(1, "Resources: R0 (Printers: 2), R1 (Scanners: 1), R2 (Disks: 2)\n");
  printf(1, "Strategy: Global Hierarchical Resource Ordering\n\n");

  init_semaphores();

  // Fork 5 processes
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int pid = fork();
    if (pid < 0) {
      printf(1, "Fork failed for process %d\n", i);
      exit();
    }
    if (pid == 0) {
      // Child process i
      run_process_work(i);
      exit();
    }
  }

  // Parent process waits for all 5 children to complete
  for (int i = 0; i < NUM_PROCESSES; i++) {
    wait();
  }

  printf(1, "\n=====================================================\n");
  printf(1, "SUCCESS: All 5 processes finished %d cycles without deadlock.\n", CYCLES);
  printf(1, "=====================================================\n");

  exit();
}
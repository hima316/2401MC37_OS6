#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_P 10
#define MAX_R 10

int N = 5; // Number of processes
int M = 3; // Number of resource types

// State matrices
int Allocation[MAX_P][MAX_R];
int Max[MAX_P][MAX_R];
int Available[MAX_R];
int Need[MAX_P][MAX_R];

// Helper to print current state
void print_state() {
  printf(1, "\n-----------------------------------------------------\n");
  printf(1, "PID  |  Allocation  |      Max     |     Need     \n");
  printf(1, "-----+--------------+--------------+-------------\n");
  for (int i = 0; i < N; i++) {
    printf(1, "P%d   |  ", i);
    for (int j = 0; j < M; j++) printf(1, "%d ", Allocation[i][j]);
    printf(1, "       |  ");
    for (int j = 0; j < M; j++) printf(1, "%d ", Max[i][j]);
    printf(1, "       |  ");
    for (int j = 0; j < M; j++) printf(1, "%d ", Need[i][j]);
    printf(1, "\n");
  }
  printf(1, "\nAvailable: ");
  for (int j = 0; j < M; j++) {
    printf(1, "%d ", Available[j]);
  }
  printf(1, "\n-----------------------------------------------------\n");
}

// Compute Need matrix: Need[i][j] = Max[i][j] - Allocation[i][j]
void calculate_need() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      Need[i][j] = Max[i][j] - Allocation[i][j];
    }
  }
}

// Safety Algorithm: Determines if system is in a safe state
int is_safe(int safe_seq[]) {
  int Work[MAX_R];
  int Finish[MAX_P];

  // 1. Initialize Work and Finish
  for (int j = 0; j < M; j++) Work[j] = Available[j];
  for (int i = 0; i < N; i++) Finish[i] = 0;

  int count = 0;
  while (count < N) {
    int found = 0;
    for (int i = 0; i < N; i++) {
      if (!Finish[i]) {
        int can_allocate = 1;
        for (int j = 0; j < M; j++) {
          if (Need[i][j] > Work[j]) {
            can_allocate = 0;
            break;
          }
        }

        // If all needs of process i can be met
        if (can_allocate) {
          for (int j = 0; j < M; j++) {
            Work[j] += Allocation[i][j];
          }
          safe_seq[count++] = i;
          Finish[i] = 1;
          found = 1;
        }
      }
    }

    // If no process could be safely executed in this iteration, system is unsafe
    if (!found) {
      return 0;
    }
  }

  return 1;
}

// Resource Request Algorithm
int request_resources(int pid, int req[]) {
  printf(1, "\n>>> Process P%d requesting resources: [ ", pid);
  for (int j = 0; j < M; j++) printf(1, "%d ", req[j]);
  printf(1, "]\n");

  // Step 1: Check if Request <= Need
  for (int j = 0; j < M; j++) {
    if (req[j] > Need[pid][j]) {
      printf(1, "ERROR: Process P%d exceeded its maximum claim!\n", pid);
      return 0;
    }
  }

  // Step 2: Check if Request <= Available
  for (int j = 0; j < M; j++) {
    if (req[j] > Available[j]) {
      printf(1, "Process P%d must wait. Resources unavailable.\n", pid);
      return 0;
    }
  }

  // Step 3: Pretend to allocate requested resources
  for (int j = 0; j < M; j++) {
    Available[j] -= req[j];
    Allocation[pid][j] += req[j];
    Need[pid][j] -= req[j];
  }

  // Step 4: Check safety of new state
  int safe_seq[MAX_P];
  if (is_safe(safe_seq)) {
    printf(1, "Request GRANTED. System remains in a SAFE state.\n");
    printf(1, "Safe sequence: ");
    for (int i = 0; i < N; i++) {
      printf(1, "P%d%s", safe_seq[i], (i == N - 1) ? "" : " -> ");
    }
    printf(1, "\n");
    return 1;
  } else {
    printf(1, "Request DENIED - would lead to an UNSAFE state. Rolling back allocation.\n");
    // Rollback changes
    for (int j = 0; j < M; j++) {
      Available[j] += req[j];
      Allocation[pid][j] -= req[j];
      Need[pid][j] += req[j];
    }
    return 0;
  }
}

// Hardcode Classic 5 Process / 3 Resource Example
void init_textbook_example() {
  N = 5;
  M = 3;

  int alloc_data[5][3] = {
    {0, 1, 0}, // P0
    {2, 0, 0}, // P1
    {3, 0, 2}, // P2
    {2, 1, 1}, // P3
    {0, 0, 2}  // P4
  };

  int max_data[5][3] = {
    {7, 5, 3}, // P0
    {3, 2, 2}, // P1
    {9, 0, 2}, // P2
    {2, 2, 2}, // P3
    {4, 3, 3}  // P4
  };

  int avail_data[3] = {3, 3, 2}; // A, B, C

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      Allocation[i][j] = alloc_data[i][j];
      Max[i][j] = max_data[i][j];
    }
  }

  for (int j = 0; j < M; j++) {
    Available[j] = avail_data[j];
  }

  calculate_need();
}

int main(int argc, char *argv[]) {
  printf(1, "=== BANKER'S ALGORITHM SIMULATION (xv6) ===\n");

  init_textbook_example();
  print_state();

  // Test 1: Initial Safety Check
  int safe_seq[MAX_P];
  if (is_safe(safe_seq)) {
    printf(1, "Initial State is SAFE.\nSafe Sequence: ");
    for (int i = 0; i < N; i++) {
      printf(1, "P%d%s", safe_seq[i], (i == N - 1) ? "" : " -> ");
    }
    printf(1, "\n");
  } else {
    printf(1, "Initial State is UNSAFE.\n");
    exit();
  }

  // Test 2: Scenario A - Safe Request (P1 requests [1, 0, 2])
  printf(1, "\n=====================================================");
  printf(1, "\nSCENARIO 1: Testing Safe Resource Request");
  printf(1, "\n=====================================================");
  int req_safe[3] = {1, 0, 2};
  request_resources(1, req_safe);
  print_state();

  // Test 3: Scenario B - Unsafe Request (P4 requests [3, 3, 0])
  printf(1, "\n=====================================================");
  printf(1, "\nSCENARIO 2: Testing Unsafe Resource Request");
  printf(1, "\n=====================================================");
  int req_unsafe[3] = {3, 3, 0};
  request_resources(4, req_unsafe);
  print_state();

  exit();
}
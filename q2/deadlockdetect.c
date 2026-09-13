#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_P 10
#define MAX_R 10

int N; // Number of processes
int M; // Number of resource types

// State matrices
int Allocation[MAX_P][MAX_R]; // Process holding resources
int Request[MAX_P][MAX_R];    // Process waiting for resources

// Wait-For Graph Adjacency Matrix
// WFG[i][j] = 1 means P_i is waiting for a resource held by P_j
int WFG[MAX_P][MAX_P];

// DFS tracking arrays for cycle detection
int visited[MAX_P];
int recStack[MAX_P];
int parent[MAX_P];

// Print Matrices and Wait-For Graph
void print_graph_info() {
  printf(1, "\n-----------------------------------------------------\n");
  printf(1, "PID  |  Allocation  |     Request    \n");
  printf(1, "-----+--------------+--------------------------------\n");
  for (int i = 0; i < N; i++) {
    printf(1, "P%d   |  ", i);
    for (int j = 0; j < M; j++) printf(1, "%d ", Allocation[i][j]);
    printf(1, "       |  ");
    for (int j = 0; j < M; j++) printf(1, "%d ", Request[i][j]);
    printf(1, "\n");
  }
  printf(1, "-----------------------------------------------------\n");

  printf(1, "\nConstructed Wait-For Graph (Edges Pi -> Pj):\n");
  int edge_count = 0;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (WFG[i][j]) {
        printf(1, "  P%d -> P%d\n", i, j);
        edge_count++;
      }
    }
  }
  if (edge_count == 0) {
    printf(1, "  (No edges found in wait-for graph)\n");
  }
  printf(1, "-----------------------------------------------------\n");
}

// Build Wait-For Graph from Allocation and Request matrices
// An edge P_i -> P_j exists if P_i requests resource R_k which is allocated to P_j
void construct_wait_for_graph() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      WFG[i][j] = 0;
    }
  }

  for (int i = 0; i < N; i++) {           // For each requesting process P_i
    for (int k = 0; k < M; k++) {         // For each resource type R_k
      if (Request[i][k] > 0) {            // P_i wants R_k
        for (int j = 0; j < N; j++) {     // Find who holds R_k
          if (i != j && Allocation[j][k] > 0) {
            WFG[i][j] = 1;                // Edge P_i -> P_j
          }
        }
      }
    }
  }
}

// DFS Helper function for Cycle Detection
int dfs_find_cycle(int u, int *cycle_start, int *cycle_end) {
  visited[u] = 1;
  recStack[u] = 1;

  for (int v = 0; v < N; v++) {
    if (WFG[u][v]) {
      if (!visited[v]) {
        parent[v] = u;
        if (dfs_find_cycle(v, cycle_start, cycle_end))
          return 1;
      } else if (recStack[v]) {
        // Back edge found: cycle detected!
        *cycle_start = v;
        *cycle_end = u;
        return 1;
      }
    }
  }

  recStack[u] = 0;
  return 0;
}

// Detect Deadlock and print the exact cycle sequence
void detect_deadlock() {
  for (int i = 0; i < N; i++) {
    visited[i] = 0;
    recStack[i] = 0;
    parent[i] = -1;
  }

  int cycle_start = -1, cycle_end = -1;
  int deadlock_found = 0;

  for (int i = 0; i < N; i++) {
    if (!visited[i]) {
      if (dfs_find_cycle(i, &cycle_start, &cycle_end)) {
        deadlock_found = 1;
        break;
      }
    }
  }

  if (deadlock_found) {
    printf(1, "\n>>> RESULT: DEADLOCK DETECTED! <<<\n");
    printf(1, "Deadlock Cycle: ");

    // Reconstruct path from cycle_end back to cycle_start
    int path[MAX_P];
    int length = 0;

    int curr = cycle_end;
    path[length++] = curr;

    while (curr != cycle_start && curr != -1) {
      curr = parent[curr];
      path[length++] = curr;
    }

    // Print cycle in correct forward direction
    for (int i = length - 1; i >= 0; i--) {
      printf(1, "P%d -> ", path[i]);
    }
    printf(1, "P%d\n", cycle_start);

  } else {
    printf(1, "\n>>> RESULT: NO DEADLOCK DETECTED (Acyclic Graph) <<<\n");
  }
}

// Scenario 1: Acyclic Wait-For Graph (No Deadlock)
void load_scenario_no_deadlock() {
  N = 4;
  M = 3;

  int alloc_data[4][3] = {
    {1, 0, 0}, // P0 holds R0
    {0, 1, 0}, // P1 holds R1
    {0, 0, 1}, // P2 holds R2
    {0, 0, 0}  // P3 holds nothing
  };

  int req_data[4][3] = {
    {0, 1, 0}, // P0 wants R1 (P1)
    {0, 0, 1}, // P1 wants R2 (P2)
    {0, 0, 0}, // P2 wants nothing (can finish)
    {1, 0, 0}  // P3 wants R0 (P0)
  };

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      Allocation[i][j] = alloc_data[i][j];
      Request[i][j] = req_data[i][j];
    }
  }
}

// Scenario 2: 3-Process Circular Deadlock (P0 -> P1 -> P2 -> P0)
void load_scenario_deadlock_3_processes() {
  N = 4;
  M = 4;

  int alloc_data[4][4] = {
    {1, 0, 0, 0}, // P0 holds R0
    {0, 1, 0, 0}, // P1 holds R1
    {0, 0, 1, 0}, // P2 holds R2
    {0, 0, 0, 1}  // P3 holds R3
  };

  int req_data[4][4] = {
    {0, 1, 0, 0}, // P0 wants R1 (held by P1)
    {0, 0, 1, 0}, // P1 wants R2 (held by P2)
    {1, 0, 0, 0}, // P2 wants R0 (held by P0) -> Cycle P0 -> P1 -> P2 -> P0!
    {0, 0, 1, 0}  // P3 wants R2 (held by P2) -> Attached but not part of core cycle
  };

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      Allocation[i][j] = alloc_data[i][j];
      Request[i][j] = req_data[i][j];
    }
  }
}

int main(int argc, char *argv[]) {
  printf(1, "=== DEADLOCK DETECTION VIA WAIT-FOR GRAPH (xv6) ===\n");

  // TEST 1: NO DEADLOCK
  printf(1, "\n=====================================================");
  printf(1, "\nTEST 1: Evaluating Acyclic Scenario (No Deadlock)");
  printf(1, "\n=====================================================");
  load_scenario_no_deadlock();
  construct_wait_for_graph();
  print_graph_info();
  detect_deadlock();

  // TEST 2: DEADLOCK (3-PROCESS CYCLE)
  printf(1, "\n=====================================================");
  printf(1, "\nTEST 2: Evaluating Circular Scenario (3-Process Deadlock)");
  printf(1, "\n=====================================================");
  load_scenario_deadlock_3_processes();
  construct_wait_for_graph();
  print_graph_info();
  detect_deadlock();

  exit();
}
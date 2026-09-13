# Assignment 6 

Deadlock Avoidance, Detection, and Prevention in xv6

---

## Overview of Questions

### Question 1: Banker's Algorithm Simulation (`bankers.c`)
This program simulates Dijkstra's classic Banker's Algorithm for deadlock avoidance in user space. It tracks Allocation, Max, Available, and Need matrices for 5 processes and 3 resource types. The safety algorithm determines if the current state is safe, while the resource request algorithm tests incoming requests by simulating allocation, checking overall system safety, and either committing the request or rolling it back.

### Question 2: Deadlock Detection via Resource Allocation Graph (`deadlockdetect.c`)
This program demonstrates deadlock detection by mapping process-resource relationships. It converts Allocation and Request matrices into a directed Wait-For Graph (where an edge exists from Process A to Process B if A is waiting on a resource held by B). A Depth-First Search (DFS) algorithm traverses the graph to detect cycles and prints the exact sequence of deadlocked processes if a circular dependency is found.

### Question 3: Deadlock Prevention via Resource Ordering (`resourceorder_bad.c` & `resourceorder_fixed.c`)
This question demonstrates how breaking the circular wait condition prevents deadlocks using real fork() child processes in xv6. The standard problematic version deliberately causes a deadlock hang by having Process A request Lock1 then Lock2, while Process B requests Lock2 then Lock1. The fixed version enforces a global hierarchical resource ordering policy (always grabbing Lock1 before Lock2), allowing both processes to complete smoothly without hanging.

### Question 4: Multi-Resource Synchronization & Deadlock Avoidance (`syncdeadlock.c`)
This project combines real multi-process synchronization with explicit deadlock prevention. Five distinct processes require pairs of shared resources (Printers, Scanners, and Disks) with limited instance counts managed via pipe-based semaphores. By applying hierarchical resource ordering, all five processes safely acquire and release shared resources over multiple iterations without hitting deadlocks or exceeding pool limits.

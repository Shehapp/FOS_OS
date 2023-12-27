# Memory and CPU Management Documentation

## Virtual Memory

Virtual memory is a crucial aspect of system management, utilizing both RAM and Disk resources to enhance the performance and efficiency of a computer system.

## Parallel Program Execution

Efficiently load and run multiple programs in parallel to make the most out of system resources and ensure optimal multitasking capabilities.

## Page Fault Handler

### Replacement Strategies

Implementing strategies for handling page faults, including replacement algorithms:

- FIFO (First-In-First-Out)
- LRU (Least Recently Used)

## Dynamic Allocation and Deallocation

Facilitate dynamic memory management with features such as dynamic allocation and deallocation. For example, creating new threads dynamically using constructs like `new Thread()`.

## Environment Cleanup

Ensure proper cleanup and resource release when programs or threads exit. This includes releasing allocated memory and other resources to maintain system integrity.

---

## How to Use

Provide instructions or code snippets on how to incorporate and utilize the memory and CPU management features in your project.

### Example:

```java
// Code snippet for dynamic allocation and thread creation
Thread newThread = new Thread();
// ... (additional code)

// Code snippet for releasing resources on program exit
// ...
```


# Page Replacement Algorithms in FOS

## overview 

### FIFO Page Replacement Algorithm
The `FIFO` algorithm follows the principle of replacing the oldest page in the memory. It maintains a queue of pages in memory, and when a page needs to be replaced, the page at the front of the queue (the oldest) is selected for removal.

#### Implementation

- Maintain a LIST  to keep track of the order in which pages are loaded into memory.
- When a page is brought into memory, it is added to the back of the LIST.
- When a page needs to be replaced, the page at the front of the queue is removed.

#### Advantages:

- Simple and easy to implement.
- Does not require maintaining usage history.

#### Disadvantages:

- May not perform well in scenarios where the usage pattern of pages is not reflected in their arrival order.



> Note: `FIFO` is the default schedular

### LRU Page Replacement Algorithm

`LRU` aims to replace the page that has not been used for the longest time. It keeps track of the order in which pages are accessed, and when a page needs to be replaced, the one that has not been used for the longest duration is selected.

#### Implementation

Active List:

- Represents the pages currently in use.
- Maintains the order of page access.
- New pages are added to the front of the active list when accessed.

Secondary List:

- Serves as a backup list for pages not actively in use.
- Pages are moved from the active list to the secondary list when they are not accessed frequently.

#### Advantages:
- Considers the historical usage pattern of pages.
- Generally performs well in various scenarios.
#### Disadvantages:

- Implementation can be more complex than FIFO.
 - Requires additional data structures to track access times.



```c
// to enable LRU in Fos
Fos > lru 2 
```

#  CPU Scheduler for FOS
## Overview
FOS (Fake Operating System) project  provides support for two scheduling algorithms: Round Robin `RR`, and `BSD Scheduler`
### Round Robin (RR) 
- Simple round-robin scheduling.
- Enqueues the current environment if it still exists.
- Picks the next environment from the ready queue and resets the quantum.

### Multilevel feeedback queues (BSD)

- Keep information about recent CPU usage for each environment
- Give highest priority to environment that has used the least CPU time recently.
- Interactive and I/O-bound environment will use little CPU time and remain at high priority.
- CPU-bound environment will eventually get lower priority as they accumulate CPU time.
- [Stanford.edu](https://www.scs.stanford.edu/23wi-cs212/pintos/pintos_7.html) for more info about BSD 





> Note: `RR` is the default schedular


```c
// to enable BSD by a 5 quantum & load multiple process
Fos > schedBSD 64 5
load fib 500 -10 
load ms1 1500 -5 
runall 
```


# Background 
The program initiates and schedules a workload of other processes. This User Space Process Scheduler reads a list of programs, with arguments, from standard input, starts up the programs in processes, and then schedules the processes to run concurrently in a time-sliced manner. It will also monitor the processes, keeping track of how the processes are using system resources. 

There are 5 parts to the project, each building on the other. 

**The program restricts to Linux system calls**

# Specification
## Part 1 Launches the Workload
The goal of Part 1 is to develop the first version of the USPS such that it can launch the workload and get all of the processes running together. USPS v1 will perform the following steps:

> - Read the program workload from the standard input. Each line in the file contains the name of the program and its arguments.
> - For each program, launch the program to run as a process using the fork(), execvp(), and any other required system calls – see below. To make things simpler, assume that the programs will run in the same environment as used by USPS.
> - Once all of the programs are running, wait for each process to terminate.
> - After all of the processes have terminated, the USPS exits.

## Part 2 Takes Control

Part 2 takes the first steps to enable USPS to gain control for this purpose, so taht it implements these two steps to create a USPS v2 building on USPS v1 in the following way:

> - Immediately after each process is created using fork(), the child process waits on the SIGUSR1 signal before calling execvp().
> - After all of the processes have been created and are awaiting the SIGUSR1 signal, the USPS parent process sends each program a SIGUSR1 signal to wake them up. Each process will then return from the sigwait() call and invoke execvp() to run the workload process.
> - After all of the processes have been awakened and are executing, the USPS sends each process a SIGSTOP signal to suspend it.
> - After all of the workload processes have been suspended, the USPS sends each process a SIGCONT signal to resume it.
> - Once all processes are back up and running, the USPS waits for each process to terminate. After all have terminated, USPS exits
USPS v2 demonstrates that we can control the suspension and resumption of processes.

## Part 3 Schedules Processes
Part 3 implements a scheduler that runs the processes according to certain scheduling policy. The simplest policy is to equally share the processor by giving each process the same amount of time to run. In this case, there is 1 workload process executing at any given time. After its time slice has completed, it suspends that process and start up another ready process. 

## Part 4
Part 4 will add some functionality to the USPS to gather relevant data from /proc that conveys some information about what system resources each workload process is consuming. This include something about execution time, memory used, and I/O. Part 4 would output the analyzed process information periodically as the workload programs are executing.
## Part 5
Part 5 is to implement some form of adjustable scheduler that uses process information to set process-specific time intervals. It knows something about process behavior, so that the time slice could be adjusted for each type of process. For instance, I/O-bound processes might be given a little less time and CPU-bound processes a bit more. By adjusting the time slice, it is possible that the entire workload could run more efficiently.

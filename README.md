# OS_Scheduler_Simulator
A way to simulate the processes in the scheduler of an OS

This was for a CS lab in my OS class

How to use:
compile with gcc -Wall -o scheduling -std=c99 scheduling.c
/scheduling input_file.txt #

Input File:
  line 1 = total number of processes
  each line after represents a process in form: A B C D
    A = process ID (int)
    B = CPU time (Cycles, int)
    C = I/O Blockage time (cycles, int)
    D = Arrival time (cycles, int)
   
   
 # = scheduling algorithm
  0 = First-Come-First-Serve
  1 = Round Robin with Quantum 2
  2 = Shortest Remaining Job First
  
  Output File
    #-inputfilename.txt
    inputfilename is without the extension of the file
    2 parts to the output
      timing snapshot
        at every line: cycle, state of each process
      Statistics
        finish time
        CPU utilization
        Turnaround time for each process

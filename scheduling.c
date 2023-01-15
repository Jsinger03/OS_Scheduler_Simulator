/*
Julian Singer
N16922787
JES9815
Operating Systems Lab 1
---------------------------------------
This program takes the input of a number corresponding to the desired scheduling algorithm
and using this algorithm it reads the input file and outputs the simulated secheduler for
the input values, then writes the output to a file that comes of the form
"algorithm#-input_file_name.txt", and removes the relative path if included in the command
line input before doing so, and the output file is found in the same directory as the file.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>


int main(int argc, char *argv[]){

int scheduling;
FILE * fp; //for creating the output file
char filename[100]=""; // the file name



// Check that the command line is correct
if(argc != 3){

  printf("usage:  ./scheduling alg input\n");
  printf("alg: the scheduling algorithm: 0, 1, or 2\n");
  printf("input: the processes inut file\n");
  exit(1);
}

scheduling = (int)atoi(argv[1]); // the scheduling algorithm


//Check that the file specified by the user exists and open it
if( !(fp = fopen(argv[2],"r")))
{
  printf("Cannot open file %s\n", argv[2]);
  exit(1);
}



struct process{
	int pid; //ID of the process
	int CPU_time; //CPU time of the process
	int IO_time; //IO time of the process
	int arrival_time; //arrival time of the process
	int time_left; //how many more cycles the process needs to run for
  int IO_time_left;//how many cycles the process has to be blocked for
  int turnaround;//statistic for the output
  char *state;//string describing the current state of the process
  int current_slice;//tracks how far into the quantum the file is
  int entered_ready;//tracks the most recent cycle where the process entered the ready queue
};
// struct tasks{
//   struct process processes[3]; //an array to hold processes
// };

int max_processes;
fscanf(fp, "%d", &max_processes);//read first line of the file

fgetc(fp);
//create a queue for runable and blocked
struct process task_list[max_processes];
int a, b, c, d;
for (int i = 0; i < max_processes; i++){
    fscanf(fp, "%d %d %d %d", &a, &b, &c, &d);
    task_list[i].pid = a;
    task_list[i].CPU_time= b;
    task_list[i].IO_time = c;
    task_list[i].arrival_time = d;
    task_list[i].time_left = b;
    task_list[i].IO_time_left = c;
    task_list[i].entered_ready = -1;
}


// form the output file name
sprintf(filename,"%d-%s",scheduling, argv[2]);



// close the processes file
fclose(fp);


//Open a file to write the output to
//I used the basename file from libgen.h to remove the segmentation fault issue when the input file comes reom a relative path
FILE *fptr;
char *output_name="";
char helper[100]="";
strncat(helper, filename, 2);
output_name = basename(filename);
if (strcmp(output_name, filename) != 0){
  strncat(helper, output_name, 98);
  output_name = helper;
}
fptr = fopen(output_name, "w");

//First come first serve algorithm

if( scheduling == 0)
{
  //variable to determine if CPU is idle or not
  int idle = 0;
  int idle_cycles=0;
  //running process
  struct process running = { .pid = -1 };
  //ready processes, can do with malloc after confirming everything else works
  struct process ready[max_processes];
  int ready_size=0;
  //blocked processes
  struct process blocked[max_processes];
  int blocked_size=0;
  //struct to help with priority
  struct process priority;
  //struct to help eliminate processes
	int cycles = 0;//to track cycles
	int completed = 0;//to know when to exit the loop
	while(completed!=max_processes)
	{
    //move processes from blocked to ready if needed
    for (int i = 0; i < blocked_size; i++){
      if (blocked[i].IO_time_left == 0){
        blocked[i].state = "ready";
        ready[ready_size] = blocked[i];
        ready[ready_size].entered_ready = cycles;
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[i].pid){
            task_list[j] = ready[ready_size];
          }
        }
        if (blocked_size > 1){
          blocked[i] = blocked[i+1];
          i--;
          blocked_size--;
        }
        else {
          blocked[0].pid=-1;
          blocked_size--;

        }
        ready_size++;
      }
    }
    //add processes to the ready queue if they arrive in this cycle
    for (int i = 0; i < max_processes; i++){
      if (cycles == task_list[i].arrival_time){
        //if adding two processes that arrived at the same time
        if (ready_size >= 1){
          if (ready[ready_size-1].arrival_time == task_list[i].arrival_time){
            if(ready[ready_size-1].pid > task_list[i].pid){
              priority = ready[ready_size-1];
              ready[ready_size-1] = task_list[i];
              ready[ready_size-1].state = "ready";
              ready[ready_size-1].entered_ready = cycles;
              ready[ready_size] = priority;
              ready_size++;
              task_list[i].state = "ready";
            }
            else{
              ready[ready_size] = task_list[i];
              ready_size++;
              ready[ready_size].state = "ready";
              task_list[i].state = "ready";
            }
          }
          else if (ready[ready_size-1].entered_ready == cycles){
            task_list[i].entered_ready = cycles;
            for (int j = ready_size-1; j >= 0; j--){
              if (ready[j].pid > task_list[i].pid && ready[j].entered_ready == cycles){
                priority = ready[j];
                ready[j] = task_list[i];
                ready[j].state = "ready";
                ready[j+1] = priority;
                task_list[i].state = "ready";
              }
            }
            ready_size++;
          }
          else {
            ready[ready_size] = task_list[i];
            ready_size++;
            ready[ready_size].state = "ready";
            task_list[i].state = "ready";
          }
        }
        else{
          ready[ready_size] = task_list[i];
          ready_size++;
          ready[ready_size].state = "ready";
          task_list[i].state = "ready";
        }
      }
    }
    if (running.pid == -1){
      if (ready[0].pid != -1)
      {
        if (ready_size != 0){
        running = ready[0];
        running.state = "running";
        idle = 0;
        for (int j = 0; j < max_processes; j++){
          ready[j] = ready[j+1];
        }
        if (ready_size == 1){
          ready[0].pid = -1;
          ready_size--;
        }
        else {
          ready_size--;
        }
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == running.pid){
            task_list[j] = running;
          }
        }
      }
      else{
        idle = 1;
        idle_cycles++;
      }
    }
    else{
      idle = 1;
      idle_cycles++;
    }
    }
    //if not idle, then carry out the cycle
    if (idle == 0){
      //adjust the running processes variables
      running.time_left--;
      //output stuff
      fprintf(fptr, "%d ", cycles);
      for (int i = 0; i < max_processes ; i++){
        //if process is arrived, print status
        if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
          fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
        }
      }
      fprintf(fptr, "\n");
        //handle decreasing the I/O cycles left for blocked processes
        if (blocked_size != 0){
          for (int i = 0; i < blocked_size; i++){
            blocked[i].IO_time_left--;
          }
        }

      //block the running process for I/O if half of its lifespan has occurred
      if (running.CPU_time % 2 != 0 && running.time_left == ((running.CPU_time/2)+1)){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
      }
      else if (running.time_left == (running.CPU_time/2) && running.IO_time_left != 0){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
      }
    }
//end of what happens in the current cycle
  if (idle == 1){
    fprintf(fptr, "%d ", cycles);
    for (int i = 0; i < max_processes ; i++){
      //if process is arrived, print status
      if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
        fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
      }
    }
    fprintf(fptr, "\n");
    //handle decreasing the I/O cycles left for blocked processes
    if (blocked_size != 0){
      for (int i = 0; i < blocked_size; i++){
        blocked[i].IO_time_left--;
      }
    }
  }

  cycles++;

  //if the running cycle has completed, remove it and increment completed processes
  if (running.time_left == 0){
    running.state="completed";
    completed++;
    running.turnaround = cycles - running.arrival_time;
    for (int j = 0; j < max_processes; j++){
      if (task_list[j].pid == running.pid){
        task_list[j] = running;
        running.pid = -1;
      }
    }
  }
	}
  fprintf(fptr, "\n");
  fprintf(fptr, "Finishing time: %d\n", cycles-1);
  float CPU_utilization = ((float)cycles - (float)idle_cycles)/(float)cycles;
  fprintf(fptr, "CPU utilization: %.2f\n", CPU_utilization);
  for (int i = 0; i < max_processes; i++){
    fprintf(fptr, "Process %d turnaround: %d\n", task_list[i].pid, task_list[i].turnaround);
  }
  fclose(fptr);
}

if (scheduling == 1)
{
  //variable to determine if CPU is idle or not
  int idle = 0;
  int idle_cycles=0;
  //running process
  struct process running = { .pid = -1 , .current_slice = 0};
  //ready processes, can do with malloc after confirming everything else works
  struct process ready[max_processes];
  int ready_size=0;
  //blocked processes
  struct process blocked[max_processes];
  int blocked_size=0;
  //struct to help with priority
  struct process priority;
	int cycles = 0;//to track cycles
	int completed = 0;//to know when to exit the loop
  int QUANTUM = 2;
	while(completed!=max_processes)
	{
    //move processes from blocked to ready if needed
    for (int i = 0; i < blocked_size; i++){
      if (blocked[i].IO_time_left == 0){
        blocked[i].state = "ready";
        blocked[i].current_slice = 0;
        blocked[i].entered_ready = cycles;
        ready[ready_size] = blocked[i];
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[i].pid){
            task_list[j] = ready[ready_size];
          }
        }
        if (blocked_size > 1){
          blocked[i] = blocked[i+1];
          i--;
          blocked_size--;
        }
        else {
          blocked[0].pid=-1;
          blocked_size--;

        }
        ready_size++;
      }
    }
    if (running.current_slice == QUANTUM){
      ready[ready_size] = running;
      ready[ready_size].current_slice = 0;
      ready[ready_size].state = "ready";
      ready[ready_size].entered_ready = cycles;
      if (ready_size >= 0){
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == ready[ready_size].pid){
            task_list[j] = ready[ready_size];
          }
        }

        running.pid = -1;
        running.current_slice = 0;
        if (ready_size >= 1)
        {
          if (ready[ready_size-1].IO_time_left == 0 && ready[ready_size-1].entered_ready == cycles){
            if(ready[ready_size-1].pid > ready[ready_size].pid){
              priority = ready[ready_size-1];
              ready[ready_size-1] = ready[ready_size];
              ready[ready_size] = priority;
              ready_size++;
            }
            else{
              ready_size++;
            }
          }
          else {
            ready_size++;
          }
        }
        else{
          ready_size++;
          }
      }

    }
    //add processes to the ready queue if they arrive in this cycle
    for (int i = 0; i < max_processes; i++){
      if (cycles == task_list[i].arrival_time){
        //if adding two processes that arrived at the same time
        if (ready_size >= 1){
          if (ready[ready_size-1].arrival_time == task_list[i].arrival_time){
            if(ready[ready_size-1].pid > task_list[i].pid){
              priority = ready[ready_size-1];
              ready[ready_size-1] = task_list[i];
              ready[ready_size-1].state = "ready";
              ready[ready_size] = priority;
              ready_size++;
              task_list[i].state = "ready";
            }
            else{
              ready[ready_size] = task_list[i];
              ready_size++;
              ready[ready_size].state = "ready";
              task_list[i].state = "ready";
            }
          }
          else if (ready[ready_size-1].entered_ready == cycles){
            for (int j = ready_size-1; j >= 0; j--){
              if (ready[j].pid > task_list[i].pid && ready[j].entered_ready == cycles){
                priority = ready[j];
                ready[j] = task_list[i];
                ready[j].state = "ready";
                ready[j+1] = priority;
                task_list[i].state = "ready";
              }
            }
            ready_size++;
          }
          else{
            ready[ready_size] = task_list[i];
            ready_size++;
            ready[ready_size].state = "ready";
            task_list[i].state = "ready";
          }
        }
        else{
          ready[ready_size] = task_list[i];
          ready_size++;
          ready[ready_size].state = "ready";
          task_list[i].state = "ready";
        }
      }
    }
    if (running.pid == -1){
      if (ready[0].pid != -1)
      {
        if (ready_size != 0){
        running = ready[0];
        running.state = "running";
        running.current_slice = 0;
        idle = 0;
        for (int j = 0; j < max_processes; j++){
          ready[j] = ready[j+1];
        }
        if (ready_size == 1){
          ready[0].pid = -1;
          ready_size--;
        }
        else {
          ready_size--;
        }
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == running.pid){
            task_list[j] = running;
          }
        }
      }
      else{
        idle = 1;
        idle_cycles++;
      }
    }
    else{
      idle = 1;
      idle_cycles++;
      running.time_left = -1;
    }
    }
    //if not idle, then carry out the cycle
    if (idle == 0){
      //adjust the running processes variables
      running.time_left--;
      running.current_slice++;
      //output stuff
      fprintf(fptr, "%d ", cycles);
      for (int i = 0; i < max_processes ; i++){
        //if process is arrived, print status
        if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
          fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
        }
      }
      fprintf(fptr, "\n");
        //handle decreasing the I/O cycles left for blocked processes
        if (blocked_size != 0){
          for (int i = 0; i < blocked_size; i++){
            blocked[i].IO_time_left--;
          }
        }

      //block the running process for I/O if half of its lifespan has occurred
      if (running.CPU_time % 2 != 0 && running.time_left == ((running.CPU_time/2)+1)){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        blocked[blocked_size].current_slice = 0;
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
        running.current_slice=0;
      }
      else if (running.time_left == (running.CPU_time/2) && running.IO_time_left != 0){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        blocked[blocked_size].current_slice = 0;
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
        running.current_slice=0;
      }
    }
//end of what happens in the current cycle
  if (idle == 1){
    fprintf(fptr, "%d ", cycles);
    for (int i = 0; i < max_processes ; i++){
      //if process is arrived, print status
      if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
        fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
      }
    }
    fprintf(fptr, "\n");
    //handle decreasing the I/O cycles left for blocked processes
    if (blocked_size != 0){
      for (int i = 0; i < blocked_size; i++){
        blocked[i].IO_time_left--;
      }
    }
  }
  cycles++;

  //if the running cycle has completed, remove it and increment completed processes
  if (running.time_left == 0){
    running.state="completed";
    running.current_slice = 0;
    completed++;
    running.turnaround = cycles - running.arrival_time;
    for (int j = 0; j < max_processes; j++){
      if (task_list[j].pid == running.pid){
        task_list[j] = running;
        running.pid = -1;
      }
    }
  }
	}
  fprintf(fptr, "\n");
  fprintf(fptr, "Finishing time: %d\n", cycles-1);
  float CPU_utilization = ((float)cycles - (float)idle_cycles)/(float)cycles;
  fprintf(fptr, "CPU utilization: %0.2f\n", CPU_utilization);
  for (int i = 0; i < max_processes; i++){
    fprintf(fptr, "Process %d turnaround: %d\n", task_list[i].pid, task_list[i].turnaround);
  }
  fclose(fptr);
}

if (scheduling == 2){
  //variable to determine if CPU is idle or not
  int idle = 0;
  int idle_cycles=0;
  //running process
  struct process running = { .pid = -1 };
  //ready processes
  struct process ready[max_processes];
  int ready_size=0;
  //blocked processes
  struct process blocked[max_processes];
  int blocked_size=0;
  //struct to help with priority
  struct process priority;
  //struct to help eliminate processes
	int cycles = 0;//to track cycles
	int completed = 0;//to know when to exit the loop
	while(completed!=max_processes)
	{
    //move processes from blocked to ready if needed
    for (int i = 0; i < blocked_size; i++){
      if (blocked[i].IO_time_left == 0){
        blocked[i].state = "ready";
        ready[ready_size] = blocked[i];
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[i].pid){
            task_list[j] = ready[ready_size];
          }
        }
        if (blocked_size > 1){
          blocked[i] = blocked[i+1];
          i--;
          blocked_size--;
        }
        else {
          blocked[0].pid=-1;
          blocked_size--;

        }
        ready_size++;
      }
    }
    //add processes to the ready queue if they arrive in this cycle
    for (int i = 0; i < max_processes; i++){
      if (cycles == task_list[i].arrival_time){
        task_list[i].state = "ready";
        ready[ready_size] = task_list[i];
        ready_size++;
      }
    }
    //Select the process with the shortest time remaining
    if (running.pid == -1){//if there is no process set to running after the last cycle
      //this occurs if a process completed or blocked in the last cycle
      if (ready_size == 0){//there were no processes in the ready queue
        idle = 1;
        idle_cycles++;
      }
      else{//there were processes in the ready queue
        running = ready[0];
        running.state = "running";
        for (int j = 0; j < ready_size; j++){
          if (running.time_left > ready[j].time_left){
            priority = running;
            priority.state = "ready";
            running = ready[j];
            running.state = "running";
            ready[j] = priority;
            for (int k = 0; k < max_processes; k++){
              if (ready[j].pid == task_list[k].pid){
                task_list[k] = ready[j];
              }
            }
          }
          else if (running.time_left == ready[j].time_left && running.pid > ready[j].pid){
            priority = running;
            priority.state = "ready";
            running = ready[j];
            running.state = "running";
            ready[j] = priority;
            for (int k = 0; k < max_processes; k++){
              if (ready[j].pid == task_list[k].pid){
                task_list[k] = ready[j];
              }
            }
          }
        }
        for (int k = 0; k < ready_size; k++){
          ready[k] = ready[k+1];
        }
        for (int i = 0; i < max_processes; i++){
          if (running.pid == task_list[i].pid){
            task_list[i].state = "running";
          }
        }
        ready_size--;
        idle = 0;
      }
    }
    else if(ready_size != 0){//if there was a process still set to running and there were processes in the ready queue
      idle = 0;
      for (int i = 0; i < ready_size; i++){
        if (running.time_left > ready[i].time_left){
          priority = running;
          running = ready[i];
          ready[i] = priority;
          for (int j = 0; j < max_processes; j++){
            if (running.pid == task_list[j].pid){
              task_list[j].state = "running";
            }
            if (ready[i].pid == task_list[j].pid){
              task_list[j].state = "ready";
            }
          }
        }
        else if (running.time_left == ready[i].time_left){
          if (running.pid > ready[i].pid){
            priority = running;
            running = ready[i];
            ready[i] = priority;
            for (int j = 0; j < max_processes; j++){
              if (running.pid == task_list[j].pid){
                task_list[j].state = "running";
              }
              if (ready[i].pid == task_list[j].pid){
                task_list[j].state = "ready";
              }
            }
          }
        }
        else{
          ready[i].state = "ready";

        }
      }
    }
  else {//there was a process left over from last cycle that was running but none were in the ready queue
    idle = 0;
  }
    //if not idle, then carry out the cycle
    if (idle == 0){
      //adjust the running process' variables
      running.time_left--;

      //formulate the output
      fprintf(fptr, "%d ", cycles);
      for (int i = 0; i < max_processes ; i++){
        //if process is arrived, print status
        if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
          fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
        }
      }
      fprintf(fptr, "\n");
        //handle decreasing the I/O cycles left for blocked processes
        if (blocked_size != 0){
          for (int i = 0; i < blocked_size; i++){
            blocked[i].IO_time_left--;
          }
        }

      //block the running process for I/O if half of its lifespan has occurred
      if (running.CPU_time % 2 != 0 && running.time_left == ((running.CPU_time/2)+1)){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
      }
      else if (running.time_left == (running.CPU_time/2) && running.IO_time_left != 0){
        blocked[blocked_size] = running;
        blocked[blocked_size].state = "blocked";
        for (int j = 0; j < max_processes; j++){
          if (task_list[j].pid == blocked[blocked_size].pid){
            task_list[j] = blocked[blocked_size];
          }
        }
        blocked_size++;
        running.pid = -1;
      }
    }
//end of what happens in the current cycle
  if (idle == 1){
    fprintf(fptr, "%d ", cycles);
    for (int i = 0; i < max_processes ; i++){
      //if process is arrived, print status
      if ((cycles >= task_list[i].arrival_time) && (task_list[i].time_left != 0)){
        fprintf(fptr, "%d: %s ", task_list[i].pid, task_list[i].state);
      }
    }
    fprintf(fptr, "\n");
    //handle decreasing the I/O cycles left for blocked processes
    if (blocked_size != 0){
      for (int i = 0; i < blocked_size; i++){
        blocked[i].IO_time_left--;
      }
    }
  }

  cycles++;

  //if the running cycle has completed, remove it and increment completed processes
  if (running.time_left == 0){
    running.state="completed";
    completed++;
    running.turnaround = cycles - running.arrival_time;
    for (int j = 0; j < max_processes; j++){
      if (task_list[j].pid == running.pid){
        task_list[j] = running;
        running.pid = -1;
      }
    }
  }
	}
  fprintf(fptr, "\n");
  fprintf(fptr, "Finishing time: %d\n", cycles-1);
  float CPU_utilization = ((float)cycles - (float)idle_cycles)/(float)cycles;
  fprintf(fptr, "CPU utilization: %.2f\n", CPU_utilization);
  for (int i = 0; i < max_processes; i++){
    fprintf(fptr, "Process %d turnaround: %d\n", task_list[i].pid, task_list[i].turnaround);
  }
  fclose(fptr);
}

return 0;
}

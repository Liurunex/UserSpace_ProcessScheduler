/* 
 * Author: Zhixin Liu
 * Duck ID: zhixinl
 * UO ID: 951452405
 * Authorship Statement:
 * This is my own work.
*/
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "p1fxns.h"

#define BUF_CAPACITY 1024
#define LINE_CAPACITY 64
#define DEFAULT_PROCS 25

typedef struct proc {
    pid_t id;
    int status;
    char *arg;
} Proc;

int sig_received = 0;
int nprocs = 0;
int sprocs = -1;
int index_proc = 0;
Proc *procs = NULL;

static void sig_handler(int signo) {
  if (signo == SIGUSR1) {
    sig_received++;
  }
}

static void free_argv(char **argv, int arg_Num) {
  int index = 0;
  if (argv != NULL) {
    for (index = 0; index < arg_Num; index++) {
      if (argv[index] != NULL)
        free(argv[index]);
    }   
    free(argv);
  } 
}

static void free_proc(Proc *procs) {
  int i = 0;
  if (procs != NULL) {
    for (i = 0; i < nprocs; i++) {
      if (procs[i].arg != NULL)
        free(procs[i].arg);
        procs[i].arg = NULL;
    }
    free(procs);
  }
}

static int add_proc(int id, char *command) {
  if (sprocs <0) {
    procs = (Proc *) calloc (DEFAULT_PROCS, sizeof(Proc));
    if (procs == NULL) {
      p1perror(1, "procs calloc failed");
      return -1;
    }
    sprocs = DEFAULT_PROCS;
  }
  else if (sprocs <= nprocs) {
    procs = (Proc *) realloc (procs, (sprocs + DEFAULT_PROCS) * sizeof(Proc));
    if (procs == NULL) {
      p1perror(1, "procs realloc failed");
      return -1;
    }
    sprocs += DEFAULT_PROCS;
  }

  /* fill in procs[nprocs] */
  procs[nprocs].id = id;
  procs[nprocs].status = -1;
  procs[nprocs].arg = p1strdup(command);

  nprocs++;
  return 1;
}

int main() {
  
  char buf[BUF_CAPACITY];
  int length = 0;
  
  if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
    p1putstr(1, "can't catch SIGUSR1\n");
    return 1;
  }

  while ((length = p1getline(0, buf, BUF_CAPACITY)) > 0) {
    
    /* deal with the new line char */
    if(buf[length-1] == '\n')
      buf[length-1] = '\0';

    int index_argv = 0;
    int index = 0;
    int arg_Num = 0;
    char program[LINE_CAPACITY];
    char tem[BUF_CAPACITY];
    char **argv;
    
    /* get the program name */
    p1getword(buf, 0, program); 
  
    /* get the argument length */
    while ((index = p1getword(buf, index, tem)) != -1) {
      arg_Num++;
    }
    
    /* allocate the argument **argv */
    argv = (char **) calloc ((arg_Num + 1), sizeof(char *));
    if (argv != NULL) {
      for (index = 0; index < arg_Num; index++) {
        argv[index] = (char *) calloc (LINE_CAPACITY, sizeof(char));
        if (argv[index] == NULL) {
          p1perror(1, "sub calloc() failed\n");
          free_argv(argv, (index + 1));
          exit(1);
        }     
      }
    }
    else {
      p1perror(1, "super calloc() failed\n");
      exit(1);
    }

    /* fill in the **argv */
    index = 0;
    index_argv = 0;
    while ((index = p1getword(buf, index, argv[index_argv])) != -1)
      index_argv++;    
    argv[arg_Num] = NULL;

    /* start fork() to create process and do execvp() */
    pid_t pid = fork();
    if (pid < 0) {
      p1perror(1, "Error: fork() child process failed\n");
      free_argv(argv, arg_Num);
      free_proc(procs);
      exit(1);
    }

    /* parent process */
    else if (pid > 0) {
      if (add_proc(pid, buf) < 0){
        free_argv(argv, arg_Num);
        free_proc(procs);
        exit(1);
      }
    }

    /* child process */
    else if (pid == 0) { 
      while(!sig_received) 
        sleep(1);   
      if (execvp(program, argv) < 0) {   
        p1perror(1, "Error: execvp() failed\n");
        free_argv(argv, arg_Num);
        free_proc(procs);
        exit(1); 
      }     
    }

    /* free the argv heap */
    free_argv(argv, arg_Num);
  }

  int i = 0;
  for (i = 0; i < nprocs; i++) {
    kill(procs[i].id, SIGUSR1);
  }
  
  for (i = 0; i < nprocs; i++) {
    kill(procs[i].id, SIGSTOP);
  }

  for (i = 0; i < nprocs; i++) {
    kill(procs[i].id, SIGCONT);
  }
  
  /* USPS waits */ 
  int status;
  for (i = 0; i < nprocs; i++) {
    wait(&status);
  }

  free_proc(procs);

  return 0;
}
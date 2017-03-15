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
#include <fcntl.h>

#include "p1fxns.h"

#define BUF_CAPACITY 1024
#define LINE_CAPACITY 64
#define DEFAULT_PROCS 25
#define PRINT_BLOCK 15
#define BLOCK 10
#define BASE 10
#define IOLIMIT 1000000
#define CPULIMIT 100

typedef struct proc {
    pid_t id;
    int status;
    char *arg;
    int prority;
} Proc;

int sig_received = 0;
int nprocs = 0;
int fprocs = 0;
int sprocs = -1;
int index_proc = 0;
int counter_accePeoc = 0;
Proc *procs = NULL;

static void strcopy(char target[], char source[]) {
  int i = 0;
  while (1) {
    target[i] = source[i];
    if (source[i] == '\0')
      break;
    i++;
  }
}

static void printSpace(int length) {
  int i =0;
  for (i = 0; i < length; i++)
    p1putstr(1, " ");
}

static char *digits = "0123456789";
static void itoa(int number, char str[], char target[]) {
  int n, i, length;
  char reverseNum[25];
  if (number <= 0) {
    p1perror(1, "pid is not vaild\n");
    return;
  }

  i = 0;
  n = number;
  length = p1strlen(str);
  str[length] = '/';
  length += 1;
  while (n != 0) {
    reverseNum[i] = digits[n % BASE];
    n /= 10;
    i++;  
  }
  i--;
  while (i >= 0) {
    str[length] = reverseNum[i];
    length += 1;
    i--;
  }
  str[length] = '\0';

  i = 0;
  n = p1strlen(target);
  for (i = 0; i < n; i++) 
    str[i + length] = target[i];
  length = i + length;
  str[length] = '\0';
}

static int myaoti(char str[]) {
    int num = 0; 
    int i = 0;
    while (str[i] != '\0' && str[i] != '\n') {
      num = num * 10 + (str[i] - '0');
      i++;
    }
    return num;
}

static void accessProcess() {
  int i = 0;
  counter_accePeoc += 1;
  p1putstr(1, "======= TIME SLICE: "); 
  p1putint(1, counter_accePeoc);
  p1putstr(1, " =======\n");
  p1putstr(1, "PRIORITY  PID       STATE     SIZE      IO_READ       IO_WRITE      USER_TIME      KERNEL_TIME    TOTAL_TIME     COMMAND");
  p1putstr(1, " \n");
  char stat_info[BUF_CAPACITY]; 
  char stam_info[BUF_CAPACITY];
  char io_info[BUF_CAPACITY];

  char memory[PRINT_BLOCK];
  char user_time[PRINT_BLOCK];
  char command[PRINT_BLOCK];
  char p_pid[PRINT_BLOCK];
  char pstatus[PRINT_BLOCK];
  char kernel_time[PRINT_BLOCK];
  char write_io[PRINT_BLOCK];
  char read_io[PRINT_BLOCK];
  
  for (i = 0; i < nprocs; i++) {
    if (procs[i].status != 1) {
    
      char str_stam[] = "/proc";
      char target_stam[] = "/statm";
      itoa(procs[i].id, str_stam, target_stam);
    
      char str[] = "/proc";
      char target[] = "/stat";
      itoa(procs[i].id, str, target);

      char str_io[] = "/proc";
      char target_io[] = "/io";
      itoa(procs[i].id, str_io, target_io);
      
      /* open file statm and close */
      int filedesc_stam = open(str_stam, O_RDONLY);
      if (filedesc_stam < 0) {
        p1perror(1, "open statm file failed");
        return;
      }

      if (p1getline(filedesc_stam, stam_info, BUF_CAPACITY) > 0) {
        p1getword(stam_info, 0, memory);
      }
        
      if (close(filedesc_stam) < 0) {
        p1perror(1, "Close statm file error\n");
        return;
      } 

      /* open file io and close */
      int filedesc_io = open(str_io, O_RDONLY);
      if (filedesc_io < 0) {
        p1perror(1, "open io file failed");
        return;
      }
      
      int line = 0;
      while (p1getline(filedesc_io, io_info, BUF_CAPACITY) > 0) {
        if (line == 2) {
          strcopy(read_io, io_info);
          line++;
          continue;
        }
        else if (line == 3) {
          strcopy(write_io, io_info);
          line++;
          continue;
        }
        else 
          line++;
      }
      
      int index_read = 0;
      int index_write = 0;
      char read_num[PRINT_BLOCK];
      char write_num[PRINT_BLOCK];
      while ((index_read = p1getword(read_io, index_read, read_num)) != -1) 
        ;

      while ((index_write = p1getword(write_io, index_write, write_num)) != -1)
        ;

      if (close(filedesc_io) < 0) {
        p1perror(1, "Close io file error\n");
        return;
      } 

      /* open file stat and close */
      int filedesc = open(str, O_RDONLY);
      if (filedesc < 0) {
        p1perror(1, "open stat file failed");
        return;
      }
      
      if (p1getline(filedesc, stat_info, BUF_CAPACITY) > 0) {
        int tem_index = 0;
        char tem_str[PRINT_BLOCK];
        int count = 0;
        while ((tem_index = p1getword(stat_info, tem_index, tem_str)) != -1) {
          if (count == 0) {
            strcopy(p_pid, tem_str);
            count++;
            continue;
          }
          else if (count == 1) {
            strcopy(command, tem_str);
            count++;
            continue;
          }
          else if (count == 2) {
            strcopy(pstatus, tem_str);
            count++;
            continue;
          }
          else if (count == 13) {
            strcopy(user_time, tem_str);
            count++;
            continue;
          }
          else if (count == 14) {
            strcopy(kernel_time, tem_str);
            break;
          }
          else 
            count++;
        }
      }
        
      if (close(filedesc) < 0) {
        p1perror(1, "Close stat file error\n");
        return;
      }

      int totaltime = myaoti(user_time) + myaoti(kernel_time);
      int tIO = myaoti(read_num) + myaoti(write_num);
      int prority = procs[i].prority;

      if (tIO > IOLIMIT) {
        procs[i].prority = (prority / 10);
      }
      if (totaltime > 100) {
        if (prority < 15)
          procs[i].prority = prority + 1;
      }

      p1putint(1, procs[i].prority);
      printSpace(BLOCK - 1);
      p1putstr(1, p_pid);
      printSpace(BLOCK - p1strlen(p_pid));
      p1putstr(1, pstatus);
      printSpace(BLOCK - p1strlen(pstatus));
      p1putstr(1, memory);
      printSpace(BLOCK - p1strlen(memory));
      p1putint(1, myaoti(read_num));
      printSpace(PRINT_BLOCK - p1strlen(read_num));
      p1putint(1, myaoti(write_num));
      printSpace(PRINT_BLOCK - p1strlen(write_num));
      p1putstr(1, user_time);
      printSpace(PRINT_BLOCK - p1strlen(user_time));
      p1putstr(1, kernel_time);
      printSpace(PRINT_BLOCK - p1strlen(kernel_time));
      p1putint(1, totaltime);
      printSpace(PRINT_BLOCK - p1strlen(user_time));
      p1putstr(1, command);
      p1putstr(1, "\n");
    }
  }
  p1putstr(1, "\n");
  p1putstr(1, "\n");
}

static void sig_handler(int signo) {
  if (signo == SIGUSR1) {
    sig_received++;
  }
}

static void child_handler(int signo) {
  if (signo == SIGCHLD) {
    pid_t childpid;
    int status;
    int i;
    while ((childpid = waitpid( -1, &status, WNOHANG)) > 0) {
      if (WIFEXITED(status)) {
        for (i = 0; i < nprocs; i++) {
          if (procs[i].id == childpid) {
            procs[i].status += 1;
            fprocs++;
            break;
          }
        }
      }
      else {
        p1perror(1, "Terminated with Error");
      }
    }
  }
}

static void time_handler(int signo) {
  if (signo == SIGALRM) {
    kill(procs[index_proc].id, SIGSTOP);

    int next_start = index_proc;
    next_start = (next_start + 1) % nprocs;
    
    while (procs[next_start].status == 1) {
      if(fprocs < nprocs)
        next_start = (next_start + 1) % nprocs;  
      else 
        return;  
    }

    if (procs[next_start].status == -1) {
      procs[next_start].status += 1;
      kill(procs[next_start].id, SIGUSR1); 
    }
    else {
      kill(procs[next_start].id, SIGCONT);
    }
    
    index_proc = next_start;

    accessProcess();
    
    if (fprocs < nprocs) {
      if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        p1putstr(1, "can't catch SIGUSR1\n");
        return;
      }
      if (signal(SIGALRM, time_handler) == SIG_ERR) {
        p1putstr(1, "can't catch SIGALRM\n");
        return;
      }
      if (signal(SIGCHLD, child_handler) == SIG_ERR) {
        p1putstr(1, "can't catch SIGCHLD\n");
        return;
      }

      if(procs[next_start].prority > 13)
        alarm(3);
      else if (procs[next_start].prority < 10)
        alarm(1);
      else 
        alarm(2);
    } 
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
  if (sprocs < 0) {
    procs = (Proc *) malloc (DEFAULT_PROCS * sizeof(Proc));
    if (procs == NULL) {
      p1perror(1, "procs malloc failed");
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
  procs[nprocs].prority = 10;
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

  if (signal(SIGALRM, time_handler) == SIG_ERR) {
    p1putstr(1, "can't catch SIGALRM\n");
    return 1;
  }
  
  if (signal(SIGCHLD, child_handler) == SIG_ERR) {
    p1putstr(1, "can't catch SIGCHLD\n");
    return 1;
  }
  
  while ((length = p1getline(0, buf, BUF_CAPACITY)) > 0) {
    
    /* deal with the new line char */
    if (buf[length - 1] == '\n')
      buf[length - 1] = '\0';

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
      if (add_proc(pid, buf) < 0) {
        free_argv(argv, arg_Num);
        free_proc(procs);
        exit(1);
      }
    }

    /* child process */
    else if (pid == 0) { 
      while (!sig_received) 
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
 
  alarm(1);

  /* wait for child processes finished */
  while (fprocs < nprocs) {
    sleep(1);
  }

  free_proc(procs);

  return 0;
}
// Define our includes
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

// Define variables
static volatile sig_atomic_t handle_control_c = 1;
static volatile sig_atomic_t handle_time_expiration = 1;
static char *p_error_executable = NULL;
static int semaphore_id = -1;

// Define Bakery
typedef struct {
  bool choice[20+1]; 
  int integer[20+1];
} bakery_structure;
typedef bakery_structure *p_bakery_structure;
p_bakery_structure merged_bakery = NULL;

// Remove Semaphores
void remove_semaphore(void) {
  if (semaphore_id > 0) {
    if (semctl(semaphore_id, 0, IPC_RMID) == -1) {
      fprintf(
        stderr, "Unable to destroy Semaphore: "
      );
      perror(
        p_error_executable
      );
    }
  }
}

// Build Semaphores
int build_semaphore(void) {
  key_t key_identifier;
  key_identifier = ftok(
    "./master", 
    0x1337
  );
  if (key_identifier < 0) {
    fprintf(
      stderr, 
      "Unable to get ftok: "
    );
    perror(
      p_error_executable
    );
    exit(EXIT_FAILURE);
  }
  semaphore_id = semget(
    key_identifier, 1,
    IPC_CREAT | 0600
  );
  if (semaphore_id < 0) {
    fprintf(
      stderr, "Unable to semget: "
    );
    perror (
      p_error_executable
    );
    exit(
      EXIT_FAILURE
    );
  }
  atexit(
    remove_semaphore
  );
  if (semctl(semaphore_id, 0, SETVAL, 0) < 0) {
    fprintf(
      stderr, 
      "Unable to initialize the Semaphore: "
    );
    perror(
      p_error_executable
    );
    exit(
      EXIT_FAILURE
    );
  }
  return 0;
}

// Clean Memory
void clean_memory(void);

// Build Memory
int build_memory(void) {
   int retval = 0;
   int shmid = shmget(
     0xBD1337, 
     sizeof(bakery_structure), 
     0660 | IPC_CREAT 
    );
   retval = shmid;
   if (shmid > 0) {
     merged_bakery = (
       p_bakery_structure
       )(shmat(
         shmid, 
         NULL, 
         0
        )
      );
     if ((void *) merged_bakery == (void *) -1) {
       perror(p_error_executable);
       merged_bakery = NULL;
       retval = -1;
     }
   } else {
     perror(
       p_error_executable
      );
     retval = -1;
   }
  if (retval >= 0) {
    atexit(clean_memory);
  }
  return retval;
}

// Clean Memory
void clean_memory(void) {
  static int not_cleaned = 1;
  if (not_cleaned) {
    int shmid = shmget(
      0xBD1337, 
      0, 
      0
    ); 
    if (shmid > 0) {
      if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror(
          p_error_executable
        );
        fprintf(
          stderr,
          "Unable to set shared memory for deletion\n"
        );
      }
    } else {
      perror (
        p_error_executable
      );
      fprintf(
        stderr, 
        "Unable to get key to shared memory\n"
      );
    }
  }
  not_cleaned = 0;
}

// Set Signature Int Executable
static void signature_int_executable(int a) {
    (void)a;
    handle_control_c = 0;
    fprintf(
      stderr,
      "CTRL_C caught\n"
    );
}

// Build Int Executable
int build_int_single_executable() {
  int retval = 0;
  struct sigaction newer_act, older_act;
  newer_act.sa_handler = signature_int_executable;
  sigemptyset (&newer_act.sa_mask);
  newer_act.sa_flags = 0;
  if (sigaction (SIGINT, NULL, &older_act) < 0) {
    perror(
      p_error_executable
    );
    retval = -1;
  } else if (older_act.sa_handler != SIG_IGN) {
    if (sigaction (SIGINT, &newer_act, NULL) < 0) {
      perror(
        p_error_executable
      );
      retval = -1;
    }
  } else {
    fprintf(
      stderr,
      "%s : SIGINT handler was not SIG_IGN\n",
      p_error_executable
    );
  }
  return retval;
}

// Timer Handler
void timer_int_handler(int s) {
  handle_time_expiration = 0;
}

// Build Int Timer
int build_inter_timer(int int_in_seconds) {
  int retval = 0;
  struct sigaction newer_act, older_act;
  newer_act.sa_handler = timer_int_handler;
  sigemptyset (&newer_act.sa_mask);
  newer_act.sa_flags = 0;
  if (sigaction (SIGALRM, NULL, &older_act) < 0) {
    perror(p_error_executable);
    retval = -1;
  } else if (older_act.sa_handler != SIG_IGN) {
    if (sigaction (SIGALRM, &newer_act, NULL) < 0) {
      perror (p_error_executable);
      retval = -1;
    }
  } else {
    fprintf(
      stderr,
      "%s : SIGALRM handler was not SIG_IGN\n",
      p_error_executable
    );
  }
  if (retval >= 0) {
    struct itimerval value; 
    value.it_interval.tv_sec = int_in_seconds;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    if (setitimer(ITIMER_REAL, &value, NULL) < 0) {
      fprintf(
        stderr, 
        "Unable to set up timer\n"
      );
      perror(
        p_error_executable
      );
    }
  }
  return retval;
}

// Define time
static char time_defined[20];

// Update Time
void update_time(void) {
  time_t user_current_time;
  struct tm *user_local_timeframe;
  time(
    &user_current_time
  );
  user_local_timeframe = localtime(
    &user_current_time
  );
  strftime(
    time_defined, 
    sizeof(time_defined), 
    "%H:%M:%S", 
    user_local_timeframe
  );
}

// Define ENUMS
typedef enum {
  master_start_log, 
  started_log_timeout, 
  log_interruption, 
  failed_log_fork,
  start_log, log_paused, 
  log_terminated, 
  log_process_terminate,
  log_children_complete, 
  begin_log_timeout,
  log_timedout, 
  log_exit,
  semaphores_log_created, 
  semaphores_log_deleted
} log_message;

// Define file
FILE * user_log = NULL;

// Update Log
void update_log(log_message msg, int optional_param) {
  update_time();
  fprintf(
    user_log, 
    "=> [%s]: ", 
    time_defined
  );
  switch(msg) {
    case semaphores_log_created:
      fprintf(
        user_log, 
        "Our semaphore has been created using ID %d..\n", 
        optional_param
      );
      break;
    case semaphores_log_deleted:
      fprintf(
        user_log, 
        "Our semaphore using ID %d has been deleted..\n", 
        optional_param
      );
      break;
    case master_start_log:
      fprintf(
        user_log, 
        "Initializing start..\n"
      );
      fprintf(
        user_log, 
        "There will be a total of %d processess executed..\n", 
        optional_param
      );
      break;
    case started_log_timeout:
      fprintf(
        user_log, 
        "Our timeout is set for %d seconds..\n", 
        optional_param
      );
      break;
    case begin_log_timeout:
      fprintf(
        user_log, 
        "There has been an interruption within %d seconds of execution..\n", 
        optional_param
      );
      break;
    case failed_log_fork:
      fprintf(
        user_log, 
        "There has been an issue executing our child process using ID: %d\n", 
        optional_param
      );
      break;
    case log_interruption:
      fprintf(
        user_log, 
        "The end user has invoked the execution of CTRL + C..\n"
      );
      break;
    case start_log:
      fprintf(
        user_log, 
        "Our process is now starting using ID %d ..\n", 
        optional_param
      );
      break;
    case log_paused:
      fprintf(
        user_log, 
        "Our Process ID %d has been paused..\n", 
        optional_param
      );
      break;
    case log_terminated:
      fprintf(
        user_log, 
        "Our Process ID %d has been terminated..\n",
        optional_param
      );
      break;
    case log_process_terminate:
      fprintf(
        user_log, 
        "Our process has paused with the following message: %d..\n", 
        optional_param
      );
      break;
    case log_timedout:
      fprintf(
        user_log, 
        "Our timeout has exceeded the time set, the program is now terminating..\n"
      );
      break;
    case log_children_complete:
      fprintf(
        user_log, 
        "The child processes have now been complete..\n"
      );
      break;      
    case log_exit:
      fprintf(
        user_log, 
        "Our processes are now being terminated, we are complete.\n"
      );
      break;
  }
}

// Call Log
int call_log(void) {
  int retval = 0;
  char process_log[20];
  snprintf(
    process_log, 
    sizeof(process_log), 
    "logfile"
  );
  user_log = fopen(
    process_log, 
    "w"
  );
  if (user_log == NULL) {
    perror(
      p_error_executable
    );
    retval = -1;
  }
  return retval;
}


// Run Slave
void run_slave(int int_process_integer) {
  char process_number[20];
  char * argv[3] = { 
    "slave", 
    process_number, 
    NULL 
  };
  char * Env[] = {
    NULL 
  };
  snprintf(
    process_number, 
    sizeof(process_number), 
    "%d", 
    int_process_integer
  );
  if (execve("./slave", argv, Env) < 0) {
    fprintf(
      stderr, 
      "Our execution has been terminated due to a failed response.\n"
    );
    perror(
      p_error_executable
    );
  }
}

// Calculate Total Processes
int total_processes(pid_t *process_group) {
  int int_count = 0;
  for (int i = 0; i < 20; i += 1) {
    if (process_group[i] != 0) {
      int_count += 1;
    } 
  }
  return int_count;
}

// Remove Process
void remove_process(pid_t *process_group, pid_t pid) {
  for (int i = 0; i < 20; i += 1) {
    if (process_group[i] == pid) {
      process_group[i] = 0;
      break;
    }
  }
}

// Remove Child Processes
void remove_child_processes(pid_t *process_group) {
  int wstatus = 0;
  for (int i= 0; i < 20; i += 1) {
    if (process_group[i] != 0) {
      kill(
        process_group[i], 
        SIGKILL
      );
      update_log(
        log_paused, 
        process_group[i]
      );
      waitpid(
        process_group[i], 
        &wstatus, 
        0
      );
      update_log(
        log_terminated, 
        process_group[i]
      );
      update_log(
        log_process_terminate, 
        wstatus
      );
      process_group[i] = 0;
    }
  }
}

// Print Processes
void print_processes(pid_t *process_group) {
  for (int i = 0; i < 20; i += 1) {
    if (process_group[i] != 0) {
      fprintf(
        user_log, 
        "=> [%s]: Our child process using index %d is using ID %d..\n",
        time_defined,
        i, 
        process_group[i]
      );
    }
  }
}

// Verify Completed Processes
void verify_completed_processes(pid_t *process_group) {
  int wstatus;
  int itemp;
  for (int i = 0; i < 20; i += 1) {
    if (process_group[i] != 0) {
      itemp = waitpid(
        process_group[i], 
        &wstatus,
        WNOHANG
      ); 
      if (itemp > 0) {
        update_log(
          log_terminated,
          itemp
        );
        update_log(
          log_process_terminate,
          wstatus
        );
        process_group[i] = 0;
      }
    }
  }
}
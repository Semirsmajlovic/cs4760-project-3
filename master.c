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
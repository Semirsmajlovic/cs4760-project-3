// Define our includes
#include <signal.h>
#include <stdlib.h> 
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Define our variables
char * p_error_executable = NULL;
int user_process_identifier = 0;
int user_process_id = 0;
int semaphore_id = -1;
FILE * log_file = NULL;

// Define our structure for bakery
typedef struct {
  bool choice[20+1]; 
  int integer[20+1];
} bakery_structure;
typedef bakery_structure *p_bakery_structure;
p_bakery_structure merged_bakery;

// Build Semaphore
int build_semaphore(void) {
  key_t key_identifier;
  key_identifier = ftok(
    "./master", 
    0x1337
  );
  if (key_identifier < 0) {
    printf(
      "Unable to determine the proper ftok() path for execution."
    );
    perror(
      p_error_executable
    );
    exit(EXIT_FAILURE);
  }
  semaphore_id = semget(
    key_identifier, 
    1, 
    0600
  );
  return 0;
}

// Wait on Semaphore
int semaphore_delay(void) {
  int identifier_number = semctl(
    semaphore_id, 
    0, 
    GETZCNT
  );
  struct sembuf ops_process[2];
  ops_process[0].sem_num = 0;
  ops_process[0].sem_op = 0;
  ops_process[0].sem_flg = 0;
  ops_process[1].sem_num = 0;
  ops_process[1].sem_op = 1;
  ops_process[1].sem_flg = 0;
  if (semop(semaphore_id, ops_process, 2) == -1) {
      fprintf(
        stderr, 
        "Our semophore is now waiting..."
      );
      perror(
        p_error_executable
      );
      exit(EXIT_FAILURE);
  }
  return identifier_number; 
}

// Update Semaphore
void update_semaphore(void) {
  struct sembuf so_procedure;
  so_procedure.sem_num = 0;
  so_procedure.sem_op = -1;
  so_procedure.sem_flg = 0;
  if (semop(semaphore_id, &so_procedure, 1) == -1) {
      fprintf(
        stderr, 
        "Our so procedure is now awaiting for our signal."
      );
      perror(
        p_error_executable
      );
      exit(EXIT_FAILURE);
  }
}

// Build Memory
int build_memory(void) {
   int retval = 0;
   int segment_id = shmget(
     0xBD1337, 
     sizeof(bakery_structure), 
     0
    ); 
   if (segment_id > 0) {
     merged_bakery = (
       p_bakery_structure
       )(shmat(
         segment_id, 
         NULL, 
         0
        )
      );
   }

  return retval;
}

// Wait on Bakery
int delay_bakery(void)
{
  int user_integer = 0;
  merged_bakery->choice[
    user_process_identifier
  ] = true;
  for (int i = 0; i < 20; i += 1) {
    if (merged_bakery->integer[i] > user_integer) {
      user_integer = merged_bakery->integer[i];
    }
  }
  user_integer += 1;
  merged_bakery->integer[
    user_process_identifier
  ] = user_integer;
  merged_bakery->choice[
    user_process_identifier
  ] = false;
  for (int j = 0; j < 20; j += 1) {
    while (merged_bakery->choice[j]) {}
    while ((merged_bakery->integer[j] != 0) && 
            ((merged_bakery->integer[j] < user_integer) ||
            ((merged_bakery->integer[j] == user_integer) && 
            (j < user_process_identifier))
            )
          )
    {}
  }
  return user_integer;
}

// Send Bakery Signal
void send_bakery_signal(void) {
  merged_bakery->integer[
    user_process_identifier] = 0;
}

// Randomize Time
unsigned int randomize_timeframe(void) {
  unsigned int start = random();
  start = (
    start % (5 - 1 + 1)
  ) + 1; 
  return (start);
}

// Define User Time
static char user_time[20];

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
    user_time, 
    sizeof(user_time), 
    "%H:%M:%S", 
    user_local_timeframe
  );
}

// Define User File
static char * user_file = "cstest";

// Update File
int update_file(int bakery_integer, int user_process_number) {
  int retval = 1;
  update_time();
  FILE *cstest = fopen(
    user_file,
    "a"
  );
  if (cstest != NULL) {
    fprintf(
      cstest,
      "[%s]: Queue %d File modified by process number %d\n",
      user_time, 
      bakery_integer, 
      user_process_number
    );
    fclose(cstest);
  }
  return retval;
}

// Define our ENUMS
typedef enum {
  start_log, 
  start_delay, 
  handle_log_criticals,
  exit_log_criticals,
  exit_log
} log_message;

// Update Log
void update_log(log_message msg,int optional_param) {
  update_time();
  fprintf(
    log_file, 
    "%s - ", 
    user_time
  );
  switch(msg) {
    case start_log:
      fprintf(
        log_file, 
        "Begin examing process ID %d.", 
        optional_param
      );
      break;
    case start_delay:
      fprintf(
        log_file, 
        "Waiting on our Semaphores...\n"
      );
      break;
    case handle_log_criticals:
      fprintf(
        log_file, 
        "Critical log has now started on process %d.\n", 
        optional_param
      );
      break;
    case exit_log_criticals:
      fprintf(
        log_file, 
        "The critical log is now terminating.\n"
      );
      break;
    case exit_log:
      fprintf(
        log_file, 
        "Our process %d is completed and will now terminate.\n", 
        optional_param
      );
      break;
  }
}

// Launch Log
int launch_log(int user_process_number) {
  int retval = 0;
  char user_log_file[20];
  snprintf(
    user_log_file,
    sizeof(
      user_log_file
    ), 
    "logfile.%d",
    user_process_number
  );

  log_file = fopen(
    user_log_file, 
    "w"
  );
  if(log_file == NULL) {
    perror(
      p_error_executable
    );
    retval = -1;
  }
  return retval;
}

// Main Function
int main(int argc, char *argv[]) {
  int completed = 1;
  int bakery_integer;
  user_process_id = getpid();
  setsid();
  if (argc == 2) {
    user_process_identifier = atoi(argv[1]);
    if (user_process_identifier < 1 || user_process_identifier > 20) {
      fprintf(
        stderr,
        "\nThe define process number %d is invalid. Please try another process number.\n\n",
        user_process_identifier
      );
      completed = 0;
    }
  } else {
    fprintf(
      stderr,
      "Please use the ./slave command to invoke the Slave command.\n"
    );
    completed = 0;
  }
  if (completed) {
    if (launch_log(user_process_identifier) < 0) {
      fprintf(
        stderr, 
        "There has been an issue opening up the logfile.\n"
      );
      completed = 0;
    } else {
      update_log(
        start_log, 
        user_process_identifier
      );
    }
    int perrorHeaderLength = strlen(argv[0]) + strlen(": Error") + 1;
    p_error_executable = malloc(perrorHeaderLength + 3);
    if (build_memory() < 0) {
      fprintf(stderr, "There has been an issue setting up the Shared Memory.\n");
      completed = 0;
    }
    if (build_semaphore() != 0) {
      fprintf (
        stderr, 
        "There has been an issue setting up the Semaphores.\n"
      ); 
      completed = 0;
    }
    for (int i = 0; i < 5; i += 1) {
      update_log(
        start_delay, 
        0
      );
      bakery_integer = semaphore_delay();
      update_log(
        handle_log_criticals, 
        bakery_integer
      );
      sleep(
        randomize_timeframe()
      );
      update_file(
        bakery_integer, 
        user_process_identifier
      );
      sleep(
        randomize_timeframe()
      );
      update_semaphore();
      update_log(
        exit_log_criticals, 
        0
      );
    }
  }
  update_log(exit_log, 0);
}
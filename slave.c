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
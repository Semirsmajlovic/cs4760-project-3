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
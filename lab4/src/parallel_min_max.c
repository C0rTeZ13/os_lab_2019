#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids;
int pnum = -1;
  void alarm_handler(int sig) {
  for (int i = 0; i < pnum; i++) {
    kill(child_pids[i], SIGKILL);
  }
}

int main(int argc, char **argv) {
  signal(SIGALRM, alarm_handler);
  int seed = -1;
  int array_size = -1;
  //int pnum = -1;
  bool with_files = false;
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            if (pnum <= 0) {
              printf("pipe number is a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 3:
            with_files = true;
            break;

          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout is a positive number");
              return 1;
            }
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
        
      case 'f':
        with_files = true;
        break;
      
      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  child_pids = malloc(sizeof(pid_t) * pnum);
  FILE *child_file; // my_file
  child_file = fopen("tempfile.txt", "w");
  int pipefd[2];
  pipe(pipefd);
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        struct MinMax child_min_max = GetMinMax(array, i * array_size / pnum, (i + 1) * array_size / pnum);
        // parallel somehow
        

        if (with_files) {
          // use files here
          fprintf(child_file, "%d ", child_min_max.min);
          fprintf(child_file, "%d\n", child_min_max.max);
        } else {
          // use pipe here
          close(pipefd[0]);
          write(pipefd[1], &child_min_max.min, sizeof(child_min_max.min));
          write(pipefd[1], &child_min_max.max, sizeof(child_min_max.max));
        }
        return 0;
      }
      else {
        child_pids[i] = child_pid;
      }
    if (timeout > 0) {
      alarm(timeout);
      sleep(timeout/2);
    }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  fclose(child_file);

  while (active_child_processes > 0) {
    // your code here
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  child_file = fopen("tempfile.txt", "r"); // my_file
  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      fscanf(child_file, "%d %d\n", &min, &max);
    } else {
      // read from pipes
      close(pipefd[1]);
      read(pipefd[0], &min, sizeof(min));
      read(pipefd[0], &max, sizeof(max));
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }
  fclose(child_file);
  remove("tempfile.txt");

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}

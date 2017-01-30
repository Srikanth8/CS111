#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <ctype.h>

#define BILLION 1000000000L

int isNumber(char* string);
void *threadFunction(void* n_iterations);
void add(long long *pointer, long long value);

pthread_mutex_t test_mutex; // for mutex
int test_lock = 0; // for spin-lock
int swap_lock = 0; // for compare_and_swap lock

static long long counter = 0;
static int opt_yield = 0; 
static int opt_sync = 0;

int main(int argc, char* argv[])
{
  static struct option long_options[] =
    {
      {"threads",  required_argument, 0, 't'},
      {"iterations",  required_argument, 0, 'i'},
      {"yield",  no_argument, &opt_yield, 1},
      {"sync",  required_argument, 0, 's'},
      {0, 0, 0, 0}
    };

  long n_threads = 1;
  long n_iterations = 1;
  
  int c;
  while ((c = getopt_long(argc, argv, ":t:i:s:", long_options, NULL)) != -1)
    {
      switch (c)
	{
	case 't':
	  if (isNumber(optarg))
	    n_threads = strtol(optarg, NULL, 10);
	  else
	    n_threads = 1;
	  break;
	  
	case 'i':
	  if (isNumber(optarg))
	    n_iterations = strtol(optarg, NULL, 10);
	  else
	    n_iterations = 1;
	  break;

	case 's':
	  ;int sync_arg = optarg[0];
	  if (sync_arg == 'm' || sync_arg == 's' || sync_arg == 'c')
	    opt_sync = sync_arg;
	  else
	    fprintf(stderr, "%s", "Unrecognized option for --sync!\n");
	  break;
	case ':':
	  switch (optopt)
	    {
	    case 't':
	      n_threads = 1;
	      break;
	    case 'i':
	      n_iterations = 1;
	      break;
	    case 's':
	      opt_sync = 0;
	      break;
	    }
	  break;
	}
    }

  if (opt_sync == 'm')
    if (pthread_mutex_init(&test_mutex, NULL))
      {
	fprintf(stderr, "%s", "Failed to initialize mutex!\n");
	exit(1);
      }
  
  uint64_t time_elapsed;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); // mark start time

  pthread_t* threads = malloc(n_threads*sizeof(pthread_t));
  
  int i;
  for (i=0; i < n_threads; i++)
    {
      int ret = pthread_create(&threads[i], NULL, threadFunction, (void *)&n_iterations);
      if (ret != 0){
	fprintf(stderr, "%s", "Failed to create thread!\n");
	exit(1);
      }
    }

  for (i=0; i < n_threads; i++)
    pthread_join(threads[i], NULL);
  
  clock_gettime(CLOCK_MONOTONIC, &end); // mark the end time

  time_elapsed = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

  long n_operations = 2*n_threads*n_iterations;
  fprintf(stdout, "%ld threads x %ld iterations x (add + subtract) = %ld operations\n", n_threads, n_iterations, n_operations);

  if (counter == 0)
    fprintf(stdout, "%s\n", "SUCCESS: final count = 0");
  else
    fprintf(stdout, "ERROR: final count = %lld\n", counter);

  fprintf(stdout, "Time Elapsed: %llu ns\n", (long long unsigned int)time_elapsed);
  fprintf(stdout, "Per Operation: %llu ns\n", (long long unsigned int)(time_elapsed/n_operations));

  exit(0);
}

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    pthread_yield();
  *pointer = sum;
}

void mutex_add(long long *pointer, long long value)
{
  pthread_mutex_lock(&test_mutex);
  add(pointer, value);
  pthread_mutex_unlock(&test_mutex);  
}

void spin_lock_add(long long *pointer, long long value)
{
  while(__sync_lock_test_and_set(&test_lock, 1));
  add(pointer, value);
  __sync_lock_release(&test_lock);
}

void compare_and_swap_add(long long *pointer, long long value)
{  
  while (__sync_val_compare_and_swap(&swap_lock, 0, 1));
  add(pointer, value);
  __sync_val_compare_and_swap(&swap_lock, 1, 0);
}

void *threadFunction(void* n_iterations)
{
  int iterations = *((int*)n_iterations);
  int i;

  switch (opt_sync)
    {
    case 0:
      for (i=0; i<iterations; i++)
	add(&counter, 1);
      
      for (i=0; i<iterations; i++)
	add(&counter, -1);
      break;
      
    case 'm':
      for (i=0; i<iterations; i++)
	mutex_add(&counter, 1);
      
      for (i=0; i<iterations; i++)
	mutex_add(&counter, -1);
      break;
      
    case 's':
      for (i=0; i<iterations; i++)
	spin_lock_add(&counter, 1);
      
      for (i=0; i<iterations; i++)
	spin_lock_add(&counter, -1);
      break;
      
    case 'c':
      for (i=0; i<iterations; i++)
	compare_and_swap_add(&counter, 1);
      
      for (i=0; i<iterations; i++)
	compare_and_swap_add(&counter, -1);
      break;
    }
  return NULL;
}

int isNumber(char* string)
{
  int isNumber=1;
  int i;
  for (i=0; string[i] != '\0'; i++)
    if (isdigit(string[i]) == 0 && string[i] != '0')
      isNumber = 0;
  return isNumber;
}

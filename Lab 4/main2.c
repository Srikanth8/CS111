#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "SortedList.h"

#define BILLION 1000000000L

int hash(char* c);
int lookFor(char* argument, char letter);
int isNumber(char* string);
void *threadFunction(void* thread_no);

pthread_mutex_t test_mutex; // for mutex
int test_lock = 0; // for spin-lock

int opt_yield = 0; 
int opt_sync = 0;

static SortedList_t* list; // Stores all lists
static SortedListElement_t* elements; // Stores all elements to be added to the list
static int* key_to_list; // Stores which list each corresponding key must be added to

static long n_iterations = 1;
static long list_size = 1;

int main(int argc, char* argv[])
{
  static struct option long_options[] =
    {
      {"threads",  required_argument, 0, 't'},
      {"iterations",  required_argument, 0, 'i'},
      {"yield",  required_argument, 0, 'y'},
      {"sync",  required_argument, 0, 's'},
      {"lists",  required_argument, 0, 'l'},
      {0, 0, 0, 0}
    };

  long n_threads = 1;
  
  int c;
  while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1)
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

	case 'y':
	  if (lookFor(optarg, 'i'))
	    opt_yield |= INSERT_YIELD;
	    
	  if (lookFor(optarg, 'd'))
	    opt_yield |= DELETE_YIELD;
	      
	  if (lookFor(optarg, 's'))
	    opt_yield |= SEARCH_YIELD;
	  break;
	  
        case 's':
	  ;int sync_arg = optarg[0];
	  if (sync_arg == 'm' || sync_arg == 's')
	    opt_sync = sync_arg;
	  else
	    fprintf(stderr, "%s", "Unrecognized option for --sync!\n");
	  break;

	case 'l':
	  if (isNumber(optarg))
	    list_size = strtol(optarg, NULL, 10);
	  else
	    list_size = 1;
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
	    case 'l':
	      list_size = 1;
	      break;
	    }
	  break;
	}
    }

  /* Intializing random number generator */
  time_t t;  
  srand((unsigned) time(&t));
  
  /* initializing empty list(s) */
  list = malloc(list_size*sizeof(SortedList_t));

  int p;
  for (p=0; p<list_size; p++)
    {
      list[p].key = NULL;
      list[p].next = NULL;
      list[p].prev = NULL;
    }

  int elements_size = n_threads*n_iterations;
  elements = malloc(elements_size*sizeof(SortedListElement_t));
  key_to_list = malloc(elements_size*sizeof(SortedListElement_t));
  
  int k;
  for (k=0; k<elements_size; k++)
    {
      char* key_ptr = malloc(sizeof(char));
      *key_ptr = rand()%26 + 'a';
      elements[k].key = key_ptr;
      key_to_list[k] = hash(key_ptr) % list_size;
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
      int ret = pthread_create(&threads[i], NULL, threadFunction, (void *)(intptr_t)i);
      if (ret != 0){
	fprintf(stderr, "%s", "Failed to create thread!\n");
	exit(1);
      }
    }

  for (i=0; i < n_threads; i++)
    pthread_join(threads[i], NULL);
  
  clock_gettime(CLOCK_MONOTONIC, &end); // mark the end time

  time_elapsed = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

  int listLength = 0;
  for (i=0; i < list_size; i++)
    listLength += SortedList_length(list + i);
  
  /* Print out Result and Timings */
  if (listLength == 0)
    fprintf(stdout, "%s\n", "SUCCESS: final length = 0");
  else if (listLength == -1)
    fprintf(stderr, "%s\n", "ERROR: final length = -1 (List is Corrupted!)");
  else
    fprintf(stderr, "ERROR: final length = %d\n", listLength);
  
  long n_operations = n_iterations*n_threads*n_iterations;
  fprintf(stdout, "%ld threads x %ld iterations x (ins + lookup/del) x (%ld/2 avg len) = %ld operations\n", n_threads, n_iterations, n_iterations, n_operations);

  fprintf(stdout, "Time Elapsed: %llu ns\n", (long long unsigned int)time_elapsed);
  fprintf(stdout, "Per Operation: %llu ns\n", (long long unsigned int)(time_elapsed/n_operations));

  /* Free allocated memory */
  int x;
  for (x=0; x<elements_size; x++)
    free((char *)elements[x].key);
  free(elements);
  free(list);
  exit(0);
}

void mutex_listFunction(int thread)
{
  int offset = thread*n_iterations;
  int i;

  for (i=0; i < n_iterations; i++)
    {
      pthread_mutex_lock(&test_mutex);
      SortedList_insert(list + key_to_list[offset+i], elements + offset + i);
      pthread_mutex_unlock(&test_mutex);
    }

  int listLength = 0;
  for (i=0; i < list_size; i++)
      listLength += SortedList_length(list + i);
    
  for (i=0; i < n_iterations; i++)
    {
      pthread_mutex_lock(&test_mutex);
      SortedListElement_t *toDelete = SortedList_lookup(list + key_to_list[offset+i], elements[i+offset].key);
      if (toDelete == NULL)
	continue;
      SortedList_delete(toDelete);
      pthread_mutex_unlock(&test_mutex);
    }
}

void spin_lock_listFunction(int thread)
{
  int offset = thread*n_iterations;
  int i;

  for (i=0; i < n_iterations; i++)
    {
      while(__sync_lock_test_and_set(&test_lock, 1));
      SortedList_insert(list + key_to_list[offset+i], elements + offset + i);
      __sync_lock_release(&test_lock); 
    }

  int listLength = 0;
  for (i=0; i < list_size; i++)
    listLength += SortedList_length(list + i);
  
  for (i=0; i < n_iterations; i++)
    {
      while(__sync_lock_test_and_set(&test_lock, 1));
      SortedListElement_t *toDelete = SortedList_lookup(list + key_to_list[offset+i], elements[i+offset].key);
      if (toDelete == NULL)
	continue;
      SortedList_delete(toDelete);
       __sync_lock_release(&test_lock); 
    }
}

void listFunction(int thread)
{
  int offset = thread*n_iterations;
  int i;

  for (i=0; i < n_iterations; i++)
    SortedList_insert(list + key_to_list[offset+i], elements + offset + i);

  int listLength = 0;
  for (i=0; i < list_size; i++)
      listLength += SortedList_length(list + i);

  for (i=0; i < n_iterations; i++)
    {
      SortedListElement_t *toDelete = SortedList_lookup(list + key_to_list[offset+i], elements[i+offset].key);
      if (toDelete == NULL)
	continue;
      SortedList_delete(toDelete);
    }
  
  //printf("{{{{THREAD}}}} %d\n", thread); 
  //printf("Thread%d length: %d\n", thread, listLength);
  //printf("element%d of thread%d: %c\n", i, thread, *(toDelete->key));
}

void *threadFunction(void* thread_no)
{  
  int thread = (intptr_t)thread_no;

  switch (opt_sync)
    {
    case 0:
      listFunction(thread);
      break;
    case 'm':
      mutex_listFunction(thread);
      break;
    case 's':
      spin_lock_listFunction(thread);
      break;
    }
  return NULL;
}

int hash(char* c)
{
  return (5381 + *c);
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

int lookFor(char* argument, char letter)
{
  int i;
  for (i=0; argument[i] != '\0'; i++)
      if (argument[i] == letter)
	return 1;
  return 0;
}

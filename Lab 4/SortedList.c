#define _GNU_SOURCE
#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
  /* If this is the first element to be added*/
  if (list->next == NULL)
    {
      list->next = element;
      element->prev = list;
      element->next = NULL;

      return;
    }

  /* If the element is to be added somewhere in the middle of the list*/
  SortedListElement_t *node = list->next;
  SortedListElement_t *prevNode = list;

  while(node != NULL)
    {
      if (*(node->key) > *(element->key))
	{	  
	  element->next = node;
	  element->prev = prevNode;
	  
	  prevNode->next = element;
	  node->prev = element;	 
	  
	  return;
	}

      if (opt_yield & INSERT_YIELD)
	pthread_yield();
      
      prevNode = node;
      node = node->next;
    }

  /* If the element is to be added to the end of the list*/
  element->next = NULL;
  element->prev = prevNode;

  prevNode->next = element;

  return;
}

int SortedList_delete( SortedListElement_t *element)
{
  if ( (element->next != NULL && element->next->prev != element) || (element->prev != NULL && element->prev->next != element))
    return 1;
  
  SortedListElement_t *nextNode = element->next;
  SortedListElement_t *prevNode = element->prev;

  if (opt_yield & DELETE_YIELD)
    pthread_yield();
  
  if (nextNode != NULL)
    nextNode->prev = prevNode;
  if (prevNode != NULL)
    prevNode->next = nextNode;
  
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
  SortedListElement_t *node = list->next;

  while(node != NULL)
    {
      if (*(node->key) == *key)
	return node;

      if (opt_yield & SEARCH_YIELD)
	pthread_yield();
      
      node = node->next;
    }
  return NULL;
}

int SortedList_length(SortedList_t *list)
{
  /* head should not have a prev element */
  if (list->prev != NULL)
    return -1;
  
  SortedListElement_t *node = list->next;
  SortedListElement_t *prevNode = list;
  int count = 0;
  
  while(node != NULL)
    {
      if (node->prev != prevNode || prevNode->next != node)
	return -1;

      if (opt_yield & SEARCH_YIELD)
	pthread_yield();
      
      count++;
      prevNode = node;
      node = node->next;
    }
  
  return count;
}

#include <stdio.h>

#include <stdlib.h>

#include <stdint.h>

#include "allocator.h"



#define DEADBEEF 0xDEADBEEF

#define DEAFBEAD 0xDEAFBEAD

#define TRUE 1

#define FALSE 0



typedef struct _header *headerPointer;

 

typedef struct _header {

  u_int32_t detect_corrupt;

  u_int32_t region_size;

  headerPointer next;

  headerPointer prev;

} header;



typedef unsigned char byte;

static byte *memory = NULL;

headerPointer free_list_ptr;



headerPointer splitMemory(headerPointer current);

/*

int main(int argc, char *argv[]) {

  allocator_init(1024);

  printf("size is %d\n",free_list_ptr->region_size);

  allocator_malloc(496);

  return 0;

}

*/

// Input: size - number of bytes to make available to the suballocator

// Output: none

// Precondition: Size is a power of two.

// Postcondition: `size` bytes are now available to the suballocator

//

// (If the suballocator is already initialised, this function does 

//nothing, even if it was initialisd with different size)

void allocator_init(u_int32_t size) {

  //Make sure size is a power of two

  int properSize = 1;

  while (properSize < size) {

    properSize *= 2;

  }

  size = properSize;

  

  //Call malloc with the correct size

  memory = (byte *)malloc(size);



  //Initialise the global variable and put the correct stuff in it

  free_list_ptr = (headerPointer) memory;

  free_list_ptr->detect_corrupt = DEADBEEF;

  free_list_ptr->region_size = size;

  free_list_ptr->prev = free_list_ptr;

  free_list_ptr->next = free_list_ptr;

}



// Stop the allocator, so that it can be init'ed again:

void allocator_end(void) {

  //Free the malloc'ed memory from allocator_init

  free(memory);

}



void *allocator_malloc(u_int32_t n) {

  headerPointer current = free_list_ptr;

  int found = FALSE;

  int looped = FALSE;



  //Searches for a fitting region

  while ((current != free_list_ptr || !looped) && !found) {

    looped =  TRUE;

    if (current->next == current) {

      if (current->region_size > (n + 2 * sizeof(*current))) {

    found = TRUE;

      } else {

    return NULL;

      }

    } else if (current->region_size >= (n + sizeof(*current))) {

      found = TRUE;

    } else {

      current = current->next;

    }

  }

  

  //Returns null if not matching regions were found

  if (!found) {

    printf("About to return null...\n");

    return NULL;

  }



  //Divides the memory by two constantly

  while (current->region_size >= (2 * (sizeof(*current) + n))) {

    current = (headerPointer)splitMemory(current);

  }

  

  free_list_ptr = current->next;



  //Removes the chosen region from the free list, marking its header as allocated

  current->prev->next = current->next;

  current->next->prev = current->prev;

  current->detect_corrupt = DEAFBEAD;

  

  return ((void*)current + sizeof(*current));

}



//Splits a given chunk of memory into two pieces

headerPointer splitMemory (headerPointer current) {

  void* ptr;

  ptr = (void*)(current);

  ptr += (current->region_size)/2;

  headerPointer new = ptr;

  current->region_size /= 2;

  new->region_size = current->region_size;

  new->detect_corrupt = DEADBEEF;



  current->next->prev = new;

  new->next = current->next;

  current->next = new;

  new->prev = current;



  return current;

}

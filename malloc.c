#ifndef HALLOC
#define HALLOC

#include <stdint.h>
#include <memoryapi.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define MIN_chunk_SIZE 32
#define SIG_LEN 8
#define MAX_HEAP_SIZE 262144
#define CHUNK_FREE 0
#define CHUNK_USED 1
#define MIN_HEAP_SIZE 64

typedef struct chunk{
  char sig[SIG_LEN];
  uint64_t current_index;    // Current index into the chunk list
  size_t size;               // Size of the chunk
  int state;                 // 0 for free, 1 for used
  struct chunk *next;        // Index of the next chunk in RAM
  struct chunk *prev;        // Only for free_list
} chunk;

typedef struct HEAP{
  char sig[SIG_LEN];
  void* heap_memory;
  size_t heap_size;
  size_t heap_free;
  void* heap_list;
  size_t list_size;
} HEAP;

HEAP heap;

char* generateRandomString(int);

int valloc_err = 0;

int init_heap(size_t heap_size){
  srand(time(NULL)); 
  int rndstr_len = SIG_LEN-1;
  char* rnd_str = generateRandomString(rndstr_len);

  if (!rnd_str){
    printf("Error: Fatal: init_heap() -> Could not generate random signature: generateRandomString()  -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }

  void *hlist;
  size_t hsize = heap_size;
  if (hsize == 0){
    hsize = 65536;
  } else if(hsize > MAX_HEAP_SIZE){
    hsize = MAX_HEAP_SIZE;
  } else if(hsize < MIN_HEAP_SIZE){
    hsize = MIN_HEAP_SIZE;
  }

  heap.heap_memory = VirtualAlloc(NULL, hsize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (heap.heap_memory == NULL){
    printf("Error: Fatal: init_heap() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }

  heap.heap_size = hsize;
  size_t hlist_size = (hsize/MIN_chunk_SIZE)*sizeof(chunk);
  heap.heap_list = VirtualAlloc(NULL, (size_t) hlist_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (heap.heap_list == NULL){
    printf("Error: Fatal: init_heap() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }

  heap.list_size = hlist_size;
  hlist = heap.heap_list;


  memset(hlist, 0, hlist_size);

  chunk *chunk_ptr = (chunk*)hlist;
  chunk_ptr->size = hsize;
  chunk_ptr->current_index = 0;
  chunk_ptr->next = &hlist;
  chunk_ptr->state = CHUNK_FREE;
  memcpy(chunk_ptr->sig, rnd_str, SIG_LEN);
  memcpy(heap.sig, rnd_str, SIG_LEN);

  if (VirtualFree(rnd_str, 0, MEM_RELEASE)) {
    ;
  } else {
    printf("Error: Fatal: init_heap() -> Could not deallocate memory: VirtualFree(random sig) -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }

  printf("\nSucessfully initialized heap\n");
  return 0;  // Heap initialized sucessfully!!!!
}



void* halloc(size_t size){
  size_t chunk_size = size;
  if (chunk_size == 0){
    chunk_size = MIN_chunk_SIZE;
  } else if (valloc_err == 1){
    return NULL;
  } else if(size > heap.heap_free){
    return NULL;    // Too bad, so sad get TF OUT OF HERE!!
  }



    return NULL;
}

//int add_chunk(){
//  for (int i = 0; i <= heap.list_size/32; i += sizeof(chunk)){
//
//  }
//}

void hfree(void *ptr){
  int index = (int*)ptr-(int*)heap.heap_memory;
  chunk* hlist = (chunk*)heap.heap_list;
  if (strcmp(heap.sig, hlist[index].sig)){
    ;
  }
  return;
}

int main(){
  init_heap(0);
  return 0;
}

char* generateRandomString(int length) {
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_length = strlen(charset);
    char* randomString = (char*)VirtualAlloc(NULL, (length + 1) * sizeof(char), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (randomString == NULL) {
        printf("Error: Fatal: generateRandomString() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        int index = rand() % charset_length;
        randomString[i] = charset[index];
    }
    randomString[length] = '\0'; // Null-terminate the string

    return randomString;
}

#endif

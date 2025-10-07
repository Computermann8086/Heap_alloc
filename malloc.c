#ifndef HALLOC
#define HALLOC

#include <stdint.h>
#include <memoryapi.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define MIN_CHUNK_SIZE 16
#define SIG_LEN 8
#define MAX_HEAP_SIZE 262144
#define CHUNK_FREE 0
#define CHUNK_USED 1
#define MIN_HEAP_SIZE 64

typedef struct chunk{
  char sig[SIG_LEN];
  void *data;                // Pointer into the heap memory
  size_t size;               // Size of the chunk
  int state;                 // 0 for free, 1 for used
  struct chunk *next;        // Index of the next chunk in RAM
  struct chunk *prev;        // Only for free_list
  size_t prev_size;          // Size of the prevoius chunk
} chunk;

typedef struct HEAP{
  char sig[SIG_LEN];
  void* heap_memory;
  size_t heap_size;
  size_t heap_free_size;
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
  void *hmem;
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
  hmem = heap.heap_memory;

  heap.heap_size = hsize;
  size_t hlist_size = (hsize/MIN_CHUNK_SIZE)*sizeof(chunk);
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
  chunk_ptr->data = hmem;
  chunk_ptr->next = hlist;
  chunk_ptr->state = CHUNK_FREE;
  chunk_ptr->prev_size=0;

  memcpy(chunk_ptr->sig, rnd_str, SIG_LEN);
  memcpy(heap.sig, rnd_str, SIG_LEN);

  if (VirtualFree(rnd_str, 0, MEM_RELEASE)) {
    ;
  } else {
    printf("Error: Fatal: init_heap() -> Could not deallocate memory: VirtualFree(random sig) -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }

  heap.heap_free_size = hsize;

  printf("\nSucessfully initialized heap\n");
  return 0;  // Heap initialized sucessfully!!!!
}



void* halloc(size_t size){
  size_t req_size = size;
  if (req_size == 0 || req_size < MIN_CHUNK_SIZE){
    req_size = MIN_CHUNK_SIZE;
  } else if (valloc_err || size > heap.heap_free_size){
    return NULL; // Too bad, so sad, now get TF OUT OF HERE!!
  } else{
    req_size = ((size + MIN_CHUNK_SIZE - 1) / MIN_CHUNK_SIZE) * MIN_CHUNK_SIZE;
  }
  chunk * chnk_ptr;
  chunk * chnk_save;
  chnk_save = NULL;
  for (size_t i = 0; i < heap.list_size/sizeof(chunk); i++){
    chnk_ptr = (chunk*)heap.heap_list+i;

    if (strncmp(chnk_ptr->sig, heap.sig, SIG_LEN)!=0){
      continue;   // Then just pass
    }
    if (chnk_ptr->state != 0){
      continue;
    }

    /* 
    If size of chunk is greater than request and truncated 
    chunk is greater than or equal to MIN_CHUNK_SIZE, truncate the chunk and add a new chunk right after it
    */
    if (chnk_ptr->size > req_size && chnk_ptr->size-req_size >= MIN_CHUNK_SIZE){  
      chnk_ptr->state = 1;
      chnk_save = chnk_ptr;
      chnk_ptr += 1;
      chnk_ptr->size = chnk_save->size-req_size;
      chnk_save->size = req_size;
      void *temp_data = chnk_save->data;
      chnk_ptr->data = (void*)((char*)temp_data+req_size);
      chnk_ptr->next = 0;
      chnk_ptr->state = CHUNK_FREE;
      chnk_ptr->prev_size=chnk_save->size;
      chnk_ptr->prev = chnk_save;
      memcpy(chnk_ptr->sig, chnk_save->sig, SIG_LEN);
      heap.heap_free_size -= req_size;
      break;          
        
    } else if(chnk_ptr->size >= req_size){
        chnk_ptr->state = 1;
        chnk_save = chnk_ptr;
        break;
    }
  }

  if(chnk_save){ 
    return chnk_save->data; 
  } else{
    return NULL;
  }
}


//int add_chunk(){
//  for (int i = 0; i <= heap.list_size/32; i += sizeof(chunk)){
//
//  }
//}

/*
hlist = [chunk, chunk, chunk...]

*/

void hfree(void *ptr){
  char* hmemt = (char*)heap.heap_memory;
  int offset = (int)heap.heap_size;
  void* hmem = (void*)(hmemt+offset);
  if (ptr > hmem){
    printf("Bad pointer to hfree()");
    return;
  }
  int index = (int*)ptr-(int*)heap.heap_memory;
  chunk* hlist = (chunk*)heap.heap_list;
  if (strncmp(heap.sig, hlist[index].sig, SIG_LEN)){
    ;
  }
  return;
}

int main(){
  init_heap(0);
  char *hi;
  hi = (char*)halloc(32);
  printf("Mallocing...\n");
  hi[0]  = 'H';
  hi[1]  = 'e';
  hi[2]  = 'l';
  hi[3]  = 'l';
  hi[4]  = 'o';
  hi[5]  = ' ';
  hi[6]  = 'W';
  hi[7]  = 'o';
  hi[8]  = 'r';
  hi[9] = 'l';
  hi[10] = 'd';
  hi[11] = '!';
  hi[12] = '\0';
  printf("This is the contents at the memory location: %s\n", hi);
  printf("Pointer from halloc: %p\n", hi);


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

chunk* find_best_fit(chunk* chnk_list_ptr, size_t size){
  size_t smallest = heap.heap_size;
  chunk* chptr;
  for (int i = 0; i<heap.list_size/sizeof(chunk); i++){
    chunk* chnk_ptr = heap.heap_list+i;
    if (chnk_ptr->size > smallest){
      continue;
    }
    chptr = chnk_ptr;
    smallest = chnk_ptr->size;
  }
  return chptr;
}

#endif

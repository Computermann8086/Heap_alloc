#ifndef HALLOC
#define HALLOC

#include <stdint.h>
#include <memoryapi.h>
#include <windows.h>
#include <stdio.h>

typedef struct{
  uint64_t current_index;
  size_t size;
  int state;
  uint64_t index_next;
} chunck;

typedef struct{
  void* heap_memory;
  void* heap_list;
} HEAP;

HEAP heap;


int valloc_err = 0;

int init_heap(size_t heap_size){
  size_t hsize = heap_size;
  if (hsize == 0){
    hsize = 65536;
  }
  heap.heap_memory = VirtualAlloc(NULL, hsize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (heap.heap_memory == NULL){
    printf("Error: Fatal: init_heap() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }
  size_t hlist_size = hsize/(sizeof(chunck)+32);
  heap.heap_list = VirtualAlloc(NULL, (size_t) hlist_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (heap.heap_list == NULL){
    printf("Error: Fatal: init_heap() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }
  printf("Sucessfully allocated memory");
  return 0;
}

void* halloc(size_t size){
  if (valloc_err == 1){
    return NULL;
  }


}


int main(){
  init_heap(0);
  return 0;
}
#endif

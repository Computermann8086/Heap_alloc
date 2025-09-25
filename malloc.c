#ifndef HALLOC
#define HALLOC

#include <stdint.h>
#include <memoryapi.h>
#include <windows.h>
#include <stdio.h>

#define MIN_chunk_SIZE 32

typedef struct{
  uint64_t current_index;
  size_t size;
  int state;                 // 0 for free, 1 for used
  uint64_t index_next;
} chunk;

typedef struct{
  void* heap_memory;
  size_t heap_size;
  void* heap_list;
  size_t list_size;
} HEAP;

HEAP heap;


int valloc_err = 0;

int init_heap(size_t heap_size){
  chunk blank_chunk;
  void* hmem;
  void* hlist;
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
  hmem = heap.heap_memory;
  heap.heap_size = hsize;
  size_t hlist_size = hsize/(sizeof(chunk)+32);
  heap.heap_list = VirtualAlloc(NULL, (size_t) hlist_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (heap.heap_list == NULL){
    printf("Error: Fatal: init_heap() -> Could not allocate memory: VirtualAlloc() -> %lu\n", GetLastError());
    valloc_err = 1;
    return -1;
  }
  heap.list_size = hlist_size;
  hlist = heap.heap_list;
  printf("Sucessfully allocated memory");

  chunk *chunk_ptr = (chunk*)hlist;

  return 0;
}

void* halloc(size_t size){
  size_t chunk_size = size;
  if (chunk_size == 0){
    chunk_size == MIN_chunk_SIZE;
  }
  if (valloc_err == 1){
    return NULL;
  }




}

int add_chunk(){
  for (int i = 0; i >= heap.list_size; i += sizeof(chunk)){

  }
}


int main(){
  init_heap(0);
  return 0;
}
#endif

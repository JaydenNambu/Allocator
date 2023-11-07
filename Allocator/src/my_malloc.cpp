#include <assert.h>
#include <my_malloc.h>
#include <stdio.h>
#include <sys/mman.h>

// A pointer to the head of the free list.
node_t *head = NULL;

// The heap function returns the head pointer to the free list. If the heap
// has not been allocated yet (head is NULL) it will use mmap to allocate
// a page of memory from the OS and initialize the first free node.
node_t *heap() {
  if (head == NULL) {
    // This allocates the heap and initializes the head node.
    head = (node_t *)mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);
    head->size = HEAP_SIZE - sizeof(node_t);
    head->next = NULL;
  }

  return head;
}

// Reallocates the heap.
void reset_heap() {
  if (head != NULL) {
    munmap(head, HEAP_SIZE);
    head = NULL;
    heap();
  }
}

// Returns a pointer to the head of the free list.
node_t *free_list() { return head; }

// Calculates the amount of free memory available in the heap.
size_t available_memory() {
  size_t n = 0;
  node_t *p = heap();
  while (p != NULL) {
    n += p->size;
    p = p->next;
  }
  return n;
}

// Returns the number of nodes on the free list.
int number_of_free_nodes() {
  int count = 0;
  node_t *p = heap();
  while (p != NULL) {
    count++;
    p = p->next;
  }
  return count;
}

// Prints the free list. Useful for debugging purposes.
void print_free_list() {
  node_t *p = heap();
  while (p != NULL) {
    printf("Free(%zd)", p->size);
    p = p->next;
    if (p != NULL) {
      printf("->");
    }
  }
  printf("\n");
}

// Finds a node on the free list that has enough available memory to
// allocate to a calling program. This function uses the "first-fit"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// found - the node found on the free list with enough memory to allocate
// previous - the previous node to the found node
//
void find_free(size_t size, node_t **found, node_t **previous) {
  // TODO
  node_t *current = heap();
  node_t *prev = NULL;
  size_t totalSize = size + sizeof(header_t);
  while(current != NULL){
    if(current->size >= totalSize){
      *found = current;
      *previous = prev;
      break;
    }
    prev = current;
    current = current->next;
  }
}

// Splits a found free node to accommodate an allocation request.
//
// The job of this function is to take a given free_node found from
// `find_free` and split it according to the number of bytes to allocate.
// In doing so, it will adjust the size and next pointer of the `free_block`
// as well as the `previous` node to properly adjust the free list.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// previous - the previous node to the free block
// free_block - the node on the free list to allocate from
//
// RETURNS:
// allocated - an allocated block to be returned to the calling program
//
// void split(size_t size, node_t **previous, node_t **free_block,
//            header_t **allocated) {
//   assert(*free_block != NULL);
//   // TODO
//   size_t totalSize = size + sizeof(header_t);
//   *allocated = (header_t*)free_block;
//   *free_block = (node_t*)(((char*)*free_block)+totalSize);
//   (*free_block)->size = (*free_block)->size - totalSize; 
//   if(*previous == NULL){
//     head = *free_block;
//   }else{
//     (*previous)->next = *free_block;
//   }
//   (*allocated)->size = size; 
//   (*allocated)->magic = MAGIC;
// }

void split(size_t size, node_t **previous, node_t **free_block,
           header_t **allocated) {
  assert(*free_block != NULL);
  // TODO
  node_t *old = *free_block;
  size_t totalSize = size + sizeof(header_t);
  *free_block = (node_t*)(((char*)*free_block)+totalSize);
  (*free_block)->size = old->size - totalSize;
  (*free_block)->next = old->next;
  if(*previous == NULL){
    head = *free_block;
  }else{
    node_t *prev = *previous;
    prev->next = *free_block;
  }
  *allocated = (header_t*)old;
  (*allocated)->size = size; 
  (*allocated)->magic = MAGIC;
}
// void split(size_t size, node_t **previous, node_t **free_block,
//            header_t **allocated) {
//     *free_block->size = *free_bl
//   }
// Returns a pointer to a region of memory having at least the request `size`
// bytes.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// A void pointer to the region of allocated memory
//
void *my_malloc(size_t size) {
  // TODO
  node_t* found = NULL;
  node_t* previous = NULL;
  header_t* allocated = NULL;
  find_free(size, &found, &previous);
  if(found == NULL){
    return NULL;
  }
  split(size, &previous, &found, &allocated);
  // allocated = (header_t*)(((char*)allocated)+sizeof(header_t));
  void* r = (void*)(allocated+1);
  return r;
}

// Merges adjacent nodes on the free list to reduce external fragmentation.
//
// This function will only coalesce nodes starting with `free_block`. It will
// not handle coalescing of previous nodes (we don't have previous pointers!).
//
// PARAMETERS:
// free_block - the starting node on the free list to coalesce
//
void coalesce(node_t *free_block) {
  // TODO
  size_t block_size = free_block->size + sizeof(node_t);
  node_t *cur = free_block->next;
  while(true){
    if ((node_t*)(((char*)free_block) + block_size) == cur){
        free_block->size += cur->size + sizeof(node_t);
        free_block->next = cur->next;
    }else{
      break;
    }
    cur = cur->next;
    block_size = free_block->size + sizeof(node_t);
    // block_size = cur->next->size + sizeof(node_t);
    // if ((node_t*)(((char*)cur) + block_size) == cur->next){
    //   free_block = (node_t*)(((char*)free_block)+block_size);
    //   cur->size = cur->size + cur->next->size;
    //   cur->next = cur->next->next;
    // }else{
    //   break;
    // }
  }
}

// Frees a given region of memory back to the free list.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by my_malloc
//
void my_free(void *allocated) {
  // TODO
  allocated = allocated - sizeof(header_t);
  header_t* header = (header_t*)allocated; 
  assert((*header).magic == MAGIC);
  node_t* freeNode = (node_t*)header;
  freeNode->size = header->size;
  freeNode->next = head;
  coalesce(freeNode);
  head = freeNode;
}

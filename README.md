# Allocator
A C++ file that simulates a basic malloc/free allocator using a first-fit linked list. my_malloc finds the first node on thee free list with enough free space, 
for the designated size requested in the call. If no node with enough free space is found it returns NULL. 
find_free takes in a size and double pointers to nodes found and previous. Using the double pointer the function 
assigns the node with sufficient space to the found node.
split splits a free node which has more space than is needed to be allocated and returns a pointer to the allocated block. 
The function my_malloc uses find_free and split to allocate free space and returns a pointer to the allocated block.
coalesce is a helper function to my_free. coalesce merges adjacent free nodes, it repeats until it reaches a non free block.
my_free frees a block and uses coalesce to clean up the list.

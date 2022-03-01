/*############################################################################

    COMP2017 USYD 2021 SEMSTER 1
    
    ASSIGNMENT 3 : "FRIENDSHIP ENDED WITH MALLOC"

    SID: 500611960

    VIRTUAL_ALLOC.H

#############################################################################*/

#ifndef VIRTUAL_ALLOC_H
#define VIRTUAL_ALLOC_H

#include <stddef.h>
#include <stdint.h>

/*############################################################################

    CONSTANT DEFINITIONS

        TWO_BIT_HEADER_SIZE_SHIFT: 2 bit shift used to get/set header size

        ONE_BIT_HEADER_ALLOC_MASK: 1 bit mask used to get/set allocation

        UNALLOCATION_MASK: Sets last Allocation bit of header to zero

        HEADER_OFFSET: 1 byte offset used to increment to/from block data section

        ALLOCATED: Flag to indicate allocated memory block

        UNALLOCATED: Flag to indicate unallocated memory block

        POWER_OF_TWO: Specifies a single power of two, used for resizing blocks

        DEFAULT_VALUE: Initialization value of zero

        TRUE: 1
        FALSE: 0

#############################################################################*/

#define TWO_BIT_HEADER_SIZE_SHIFT (2)
#define ONE_BIT_HEADER_ALLOC_MASK (0x01)
#define UNALLOCATION_MASK (0xFE)
#define HEADER_OFFSET (1)
#define UNALLOCATED (0)
#define ALLOCATED (1)
#define POWER_OF_TWO (1)
#define TRUE (1)
#define FALSE (0)
#define DEFAULT_VALUE (0)

struct buddy_allocator{

    /*########################################################################

        STRUCT BUDDY_ALLOCATOR

        Memory structure used for buddy-allocation of memory blocks.

        * Initial_size: Memory size of heap (2^initial_size)

        * Min_size: Smallest size memory block available in heap (2^min_size)
        
        * Heap_memory: Pointer to writable section of memory block 

    #########################################################################*/

    uint8_t initial_size;
    uint8_t min_size;
    uint8_t heap_memory[];
};


/*############################################################################

    HELPER FUNCTIONS

#############################################################################*/

int pow_2(int power);
int log_2 (uint32_t value);

void * find_best_fit(uint8_t* first_block, uint8_t size_to_allocate_log_2, 
uint8_t minsize, uint8_t maxsize);

void split_block(void* best_fit, uint8_t size_diff, 
struct buddy_allocator* buddy_allocator);

void check_next_neighbour_for_valid_buddy(void * block_to_free, 
uint8_t *block_size, struct buddy_allocator* buddy_allocator, 
void * heapstart, uint8_t * neighbour_found_next, uint8_t * neighbour_found_prev);

void * check_prev_neighbour_for_valid_buddy(void * block_to_free, 
uint8_t *block_size, struct buddy_allocator* buddy_allocator, 
void * heapstart, uint8_t * neighbour_found_next, uint8_t * neighbour_found_prev);

void free_buddies(void*block_to_free, uint8_t block_size, uint8_t block_alloc, 
    void * heapstart, struct buddy_allocator* buddy_allocator);


/*############################################################################

    MAIN FUNCTIONS

#############################################################################*/

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size);

void * virtual_malloc(void * heapstart, uint32_t size);

int virtual_free(void * heapstart, void * ptr);

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size);

void virtual_info(void * heapstart);

#endif
/*############################################################################

    COMP2017 USYD 2021 SEMSTER 1
    
    ASSIGNMENT 3 : "FRIENDSHIP ENDED WITH MALLOC"

    SID: 500611960

    VIRTUAL_ALLOC.C

#############################################################################*/

#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*############################################################################

    HELPER FUNCTIONS

#############################################################################*/

int pow_2(int power) {

    /*#########################################################################

        FUNCTION: POW_2

        Returns: Integer, 2^power

        Input: Integer, power of 2 to return

    ##########################################################################*/

    if(power == UNALLOCATED) {
        return 1;
    }
    else if(power == 1) {
        return 2;
    }
    else{
        int return_value = 2 << (power - 1);
        return return_value;
    }

}

int log_2 (uint32_t value) {

    /*########################################################################

        FUNCTION: LOG_2

        Return Ceiling of log base 2 of given value
        Ceiling is used to ensure memory block sizes remain integer powers of 2

        Input: Integer value
        Returns: Integer Ceiling of log base 2 of given value

    #########################################################################*/

    return ceil( log(value) / log(2) );
}

void * find_best_fit(uint8_t* first_block, 
    uint8_t size_to_allocate_log_2, uint8_t minsize, uint8_t maxsize) {

    /*########################################################################

        FUNCTION: FIND_BEST_FIT

        Recursively iterates over memory blocks within Buddy Structure 
        to finds best fitting block for the specified size.

        Search first tries to find perfect match block 2^size, then 2^size+1..

        Input: Pointer to first memory block, Block size to find (power of 2),
            Minimum block size (power of 2), maximum Block size (power of 2)
        
        Returns: void * to best fit block data section (If found), else NULL

    #########################################################################*/
    
    //Check for valid block size
    if(size_to_allocate_log_2 > maxsize) {
        return NULL;
    }

    //Init pointer to first block, 
    //Get first block size and alloc status
    void* curr_block_search_1 = first_block;
    uint8_t size = *first_block >> TWO_BIT_HEADER_SIZE_SHIFT;
    uint8_t alloc = *first_block & ONE_BIT_HEADER_ALLOC_MASK;

    //Search all blocks until we find an unallocated exact fit
    //or reach the end of the heap
    while((curr_block_search_1 < virtual_sbrk(0)) && ((alloc == ALLOCATED) ||
     size != size_to_allocate_log_2)) {

        void * next_block = curr_block_search_1 + pow_2(size) +
         (pow_2(size)/pow_2(minsize));

        curr_block_search_1 = next_block;
        size = *(uint8_t*)curr_block_search_1 >> TWO_BIT_HEADER_SIZE_SHIFT;
        alloc = *(uint8_t*)curr_block_search_1 & ONE_BIT_HEADER_ALLOC_MASK;

    }

    //End of search -> if we found a fit, return pointer
    if(size == size_to_allocate_log_2 && alloc == UNALLOCATED) {
        return curr_block_search_1;
    }

    //No valid fit, Recursively call function with next best fit 2^j+1
    else{
        void* best_fit = find_best_fit(first_block, size_to_allocate_log_2+1, minsize, maxsize);
        return best_fit;
    }

}

void split_block(void* best_fit, uint8_t size_diff, 
    struct buddy_allocator* buddy_allocator) {

    /*########################################################################

        FUNCTION: SPLIT_BLOCK

        Helper function for Virtual_Malloc()

        Given a best fitting block and size difference between 
        the best fitting block and the desired allocation, 
        splits the block in half until desired block size is created.

        Inputs: Pointer to best fitting block, Size difference (Power of 2),
        Pointer to buddy_allocator.

        Return: Void

    #########################################################################*/


    //Iterate for value of size difference,
    //e.g. 2^4-2^2 = Power difference of 2, therefore we split twice.
    for(int i=0; i<size_diff; i++) {

        //Get current size and new size after split
        uint8_t curr_size = DEFAULT_VALUE;
        curr_size = *((uint8_t*)best_fit) >> TWO_BIT_HEADER_SIZE_SHIFT;
        uint8_t new_size = curr_size - POWER_OF_TWO;

        //Set new size of current block
        *(uint8_t*)best_fit = (new_size << TWO_BIT_HEADER_SIZE_SHIFT);

        //how much extra header space required? 
        // (i.e. what is the max amount of blocks this new block could be divided into?)
        uint32_t extra_space_needed = 
            (uint32_t)(pow_2(new_size) / pow_2(buddy_allocator->min_size));

        //Create new block from current block
        //Set to Current block address + block size + extra header space
        void *b = (void*)((unsigned char*)best_fit + 
            pow_2(new_size) + extra_space_needed);

        //Set size of buddy block created from split
        *(uint8_t*)b = (new_size << TWO_BIT_HEADER_SIZE_SHIFT);

    }

}

void check_next_neighbour_for_valid_buddy(void * block_to_free, 
uint8_t *block_size, struct buddy_allocator* buddy_allocator, 
void * heapstart, uint8_t * neighbour_found_next, uint8_t * neighbour_found_prev){

   /*########################################################################

        FUNCTION: CHECK NEXT NEIGHBOUR FOR VALID BUDDY

        Helper function for Virtual_Free()

        Given a block to free, Checks if next block is equal sized buddy

        Input: Pointer to block to be freed, block size, block allocation,
        pointer to heapstart, pointer to buddy_allocator

        Returns: VOID

    #########################################################################*/

    //Read in next block
    void* next_block = (block_to_free) + pow_2(*block_size) 
    + (pow_2(*block_size) / pow_2(buddy_allocator->min_size));

    uint8_t next_alloc = *(uint8_t*)next_block & ONE_BIT_HEADER_ALLOC_MASK;
    uint8_t next_size = *(uint8_t*)next_block >> TWO_BIT_HEADER_SIZE_SHIFT;

    //Check If next block is an unallocated buddy
    if(next_block < virtual_sbrk(0) && next_block > heapstart) {

        //if next block is buddy and free, combine
        if(next_alloc == UNALLOCATED && next_size == *block_size) {

            //Set flag to continue loop as we may find another buddy
            *neighbour_found_next = TRUE;

            //we found a buddy in front
            //Remove it and increment current block size
            *(uint8_t*)next_block = DEFAULT_VALUE;

            uint8_t block_to_free_old_size = 
                *(uint8_t*)block_to_free >> TWO_BIT_HEADER_SIZE_SHIFT;

            uint8_t block_to_free_new_size = block_to_free_old_size + 1;

            *(uint8_t*)block_to_free = 
                block_to_free_new_size << TWO_BIT_HEADER_SIZE_SHIFT;
            *block_size = block_to_free_new_size;

        }else{
            //No next buddy found
            *neighbour_found_next = FALSE;
        }
    }else{
        //No next buddy found
        *neighbour_found_next = FALSE;
    } 
}

void * check_prev_neighbour_for_valid_buddy(void * block_to_free, 
uint8_t *block_size, struct buddy_allocator* buddy_allocator, 
void * heapstart, uint8_t * neighbour_found_next, uint8_t * neighbour_found_prev){

   /*########################################################################

        FUNCTION: CHECK PREV NEIGHBOUR FOR VALID BUDDY

        Helper function for Virtual_Free()

        Given a block to free, Checks if previous block is equal sized buddy

        Input: Pointer to block to be freed, block size, block allocation status,
        pointer to heapstart, pointer to buddy_allocator

        Returns: Updated pointer to block_to_free

    #########################################################################*/

    //Check for Prev Buddy
    //Check that block to free is not first block
    if((void*)block_to_free <= 
        (void*)&(buddy_allocator->heap_memory[0])) {

        *neighbour_found_prev = FALSE;
        return block_to_free;

    }

    //Iterate from first block to find block before block_to_free
    void * curr_block_1 = (void*)&(buddy_allocator->heap_memory[0]);

    uint8_t curr_block_size_1 = 
        *(uint8_t*)curr_block_1 >> TWO_BIT_HEADER_SIZE_SHIFT;

    uint8_t curr_block_alloc = 
        *(uint8_t*)curr_block_1 & ONE_BIT_HEADER_ALLOC_MASK;  

    void* next_block_check = (curr_block_1) + 
    pow_2(curr_block_size_1) + (pow_2(curr_block_size_1) / pow_2(buddy_allocator->min_size));

    while(next_block_check != block_to_free && next_block_check <= block_to_free) {

        curr_block_1 = next_block_check;
        curr_block_size_1 = *(uint8_t*)curr_block_1 >> TWO_BIT_HEADER_SIZE_SHIFT;
        curr_block_alloc = *(uint8_t*)curr_block_1 & ONE_BIT_HEADER_ALLOC_MASK;   

        next_block_check = (curr_block_1) + pow_2(curr_block_size_1) + 
        (pow_2(curr_block_size_1) / pow_2(buddy_allocator->min_size));

    }

    //We should now be pointing at the block before the block to free
    if(next_block_check == block_to_free) {

        //Check previous block is unallocated and of equal size
        if(curr_block_alloc == UNALLOCATED && curr_block_size_1 == *block_size) {

            

            //Set flag to indicate an unallocated buddy was found
            *neighbour_found_prev = TRUE;

            //Merge blocks
            *(uint8_t*)block_to_free = DEFAULT_VALUE;
            block_to_free = curr_block_1;
            uint8_t old_size = *(uint8_t*)curr_block_1 >> TWO_BIT_HEADER_SIZE_SHIFT;
            uint8_t new_size = old_size + POWER_OF_TWO;
            *(uint8_t*)block_to_free = (new_size << TWO_BIT_HEADER_SIZE_SHIFT);
            *(uint8_t*)block_to_free = *(uint8_t*)block_to_free & 0xFE;
            *block_size = new_size;

        }else{
        //No prev buddy found
            *neighbour_found_prev = FALSE;
        }
    }else{
        //No prev buddy found
        *neighbour_found_prev = FALSE;
    }
    return block_to_free;
}

void free_buddies(void*block_to_free, uint8_t block_size, uint8_t block_alloc, 
    void * heapstart, struct buddy_allocator* buddy_allocator) {

    /*########################################################################

        FUNCTION: FREE BUDDIES

        Helper function for Virtual_Free()

        Given a block to free, Iteratively checks relevant buddy blocks,
        Freeing unallocated buddy's of same size until no buddy is available
        to be freed

        Input: Pointer to block to be freed, block size, block allocation status,
        pointer to heapstart, pointer to buddy_allocator

        Returns: VOID

    #########################################################################*/

    //Set flags loop condition, neighbour found next/prev
    uint8_t neighbour_found_next = TRUE;
    uint8_t neighbour_found_prev = TRUE;


    //Iterate until no buddy's found
    while(neighbour_found_next || neighbour_found_prev) {

        //free block requiring freeing (remove last bit)
        *(uint8_t*)block_to_free = 
            *(uint8_t*)block_to_free & UNALLOCATION_MASK;

        //Initialize flag for block order check
        uint8_t even_block = FALSE;

        //Check if block to free is first block
        if((void*)block_to_free <= 
            (void*)&(buddy_allocator->heap_memory[0])) {

            even_block = TRUE;
        }
        else{
        uint8_t block_count = 1;

        //Iterate from first block to find block before block_to_free
        void * curr_block_1 = 
            (void*)&(buddy_allocator->heap_memory[0]);

        uint8_t curr_block_size_1 = 
            *(uint8_t*)curr_block_1 >> TWO_BIT_HEADER_SIZE_SHIFT;   

        void* next_block_check = (curr_block_1) + 
        pow_2(curr_block_size_1) + 
            (pow_2(curr_block_size_1) / pow_2(buddy_allocator->min_size));

        while(next_block_check != block_to_free && next_block_check <= block_to_free) {

            //increment block count
            block_count += 1;

            curr_block_1 = next_block_check;
            curr_block_size_1 = 
                *(uint8_t*)curr_block_1 >> TWO_BIT_HEADER_SIZE_SHIFT;

            next_block_check = (curr_block_1) + pow_2(curr_block_size_1) + 
            (pow_2(curr_block_size_1) / pow_2(buddy_allocator->min_size));

        }

        //check if block to free is even or odd block in sequence
        if(block_count % 2 == 1){
            even_block = FALSE;
        }else{
            even_block = TRUE;
        }

    }

    //Block count will now represent the number of the block in the sequence
    //If block is "even" -> block zero, 2, 4, etc
    //Block's buddy must be checked NEXT first
    if(even_block){

        check_next_neighbour_for_valid_buddy(block_to_free, &block_size, buddy_allocator, heapstart, &neighbour_found_next, 
        &neighbour_found_prev);

        if(neighbour_found_next == TRUE){
            continue;
        }

        block_to_free = check_prev_neighbour_for_valid_buddy(block_to_free, &block_size, buddy_allocator, heapstart, &neighbour_found_next, 
        &neighbour_found_prev);
    }

    //If block is "odd" -> block 1, 3, 5, etc
    //Blocks buddy must be checked PREV first
    else{
        block_to_free = check_prev_neighbour_for_valid_buddy(block_to_free, &block_size, buddy_allocator, heapstart, &neighbour_found_next, 
        &neighbour_found_prev);

        if(neighbour_found_prev == TRUE){
            continue;
        }

        check_next_neighbour_for_valid_buddy(block_to_free, &block_size, buddy_allocator, heapstart, &neighbour_found_next, 
        &neighbour_found_prev);
    }
    }
}

/*############################################################################

    MAIN FUNCTIONS

#############################################################################*/


void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {


    /*########################################################################

        FUNCTION: INIT ALLOCATOR

        Initialises buddy allocation data structure

        Buddy allocator starts with an initial unallocated block of memory 
            of 2initial_size bytes. 
        The minimum size of allocatable blocks will be 2min_size

        Input: Pointer to heapstart, initial size (log2), min size (log2)

        Return: VOID

    #########################################################################*/


    uint32_t available_space = DEFAULT_VALUE;
    uint32_t number_of_blocks_max = DEFAULT_VALUE;
    uint32_t required_space = DEFAULT_VALUE;

    //Check heapstart is not NULL
    if(heapstart == NULL) {
        perror("NULL heapstart!\n");
        exit(1);
    }

    //Check Min < Init
    if(min_size > initial_size) {
    perror("Min size of heap > initial size, aborting!\n");
    exit(1);   
    }

    //Adjust heap size according to given sbrk
    void * heapend = virtual_sbrk(0);

    //Check available space
    available_space = heapend - heapstart;

    //How many blocks will we store maximum?
    number_of_blocks_max = pow_2(initial_size) / pow_2(min_size);

    //Calculte space needed (buddy allocator + max blocks + initial size)
    required_space = sizeof(struct buddy_allocator) +
         (number_of_blocks_max) + pow_2(initial_size) ;

    //if we have too much space, we need to shrink heap space
    if(available_space > required_space) {
        uint32_t excess_space = DEFAULT_VALUE;
        excess_space = available_space - required_space;
        virtual_sbrk(-excess_space);
    }

    //if we don't have enough space, we need to increment our virtual heap!
    else if(available_space < required_space) {
        uint32_t space_increase_needed = 
            required_space - available_space;
        virtual_sbrk(space_increase_needed);
    }

    //initialize buddy_allocator struct
    struct buddy_allocator * heap_buddy_allocator = heapstart;
    heap_buddy_allocator->initial_size = initial_size;
    heap_buddy_allocator->min_size = min_size;

    //set next available byte after buddy allocator to be the initial block
    heap_buddy_allocator->heap_memory[0] = 
        (initial_size << TWO_BIT_HEADER_SIZE_SHIFT);

}

void * virtual_malloc(void * heapstart, uint32_t size) {

    /*########################################################################

        FUNCTION: VIRTUAL MALLOC

        Allocates and returns leftmost unallocated block of specified size
        from virtual heap memory

        Splits blocks if required, finds best fit block.

        Input: Pointer to heapstart, size of block to allocate (log2)

        Returns: Void pointer to allocated block or NULL if no block available

    #########################################################################*/

    //Point to buddy_allocator structure
    struct buddy_allocator* buddy_allocator = heapstart;

    //calculate size in power of two
    uint8_t size_to_allocate_log_2 = DEFAULT_VALUE;

    //no size to allocate
    if (size == UNALLOCATED || size < DEFAULT_VALUE) {
        perror("invalid size passed to malloc! must be at least 1 byte");
        return NULL;
    }
    else{
        //take log base 2 of size
        size_to_allocate_log_2 = log_2(size);
        //Check size to allocate is not greater than initial size
        if(size_to_allocate_log_2 > buddy_allocator->initial_size) {
            return NULL;
        }
    }

    //If given size is smaller than min size, set to min size.
    if(size_to_allocate_log_2 < buddy_allocator->min_size) {
        size_to_allocate_log_2 = buddy_allocator->min_size;
    }

    //Point to first block
    void * first_block = &(buddy_allocator->heap_memory[0]);

    //Find best fit block
    void * best_fit = find_best_fit(first_block, size_to_allocate_log_2, 
    buddy_allocator->min_size, buddy_allocator->initial_size);

    //If unable to find best fit, return NULL
    if(best_fit == NULL) {
        return NULL;
    }

    //Get best fit size and allocation status
    uint8_t best_fit_size = 
        *(uint8_t*)best_fit >> TWO_BIT_HEADER_SIZE_SHIFT;

    uint8_t best_fit_alloc = 
        *(uint8_t*)best_fit & ONE_BIT_HEADER_ALLOC_MASK;
    
    //If best fit block is unallocated and matches size, return it 
    if(best_fit_size == size_to_allocate_log_2 && 
        best_fit_alloc == UNALLOCATED) {

        *(uint8_t*)best_fit = *(uint8_t*)best_fit | ONE_BIT_HEADER_ALLOC_MASK;
        return (void*)(best_fit)+ HEADER_OFFSET;

    }

    //If best fit block is larger than allocation size, split it
    else if(best_fit_size > size_to_allocate_log_2 && 
        best_fit_alloc == UNALLOCATED) {

        //Calculate power difference between best fit and desired size
        int32_t size_diff = DEFAULT_VALUE;
        size_diff = best_fit_size - size_to_allocate_log_2;

        //if the different is greater than 0, 
        //e.g. we have 2^4 available but only need 2^3
        //halve for size_diff iterations 
        //update each blocks size and allocation
        if(size_diff > DEFAULT_VALUE) {
            split_block(best_fit, size_diff, buddy_allocator);
        }

        //set allocation of freshly malloc'd block
        *(uint8_t*)best_fit = 
            *((uint8_t*)best_fit) | ONE_BIT_HEADER_ALLOC_MASK;

        //Return writable section of block
        return (void*)(best_fit)+1;

    }else{
        //No block available
        return NULL;
    }

return NULL;

}

int virtual_free(void * heapstart, void * ptr) {

    /*########################################################################

        FUNCTION: VIRTUAL FREE

        Given an allocated block of memory, frees the block and its buddy's

        Input: Pointer to heapstart, Pointer to block_to_free

        Returns: 1 if error, 0 is free successful

    #########################################################################*/


    //Initialize buddy allocator struct
    struct buddy_allocator* buddy_allocator = heapstart;

    // Check PTR to be freed is not NULL
    if(ptr == NULL) {
        return 1;
    }

    //Initialize pointer to header of block to be freed
    void* block_to_free = (void*)((char*)ptr-HEADER_OFFSET);

    //Check block to free is located within heap
    if(block_to_free >= virtual_sbrk(0) || block_to_free < heapstart) {
        return 1;
    }

    //Get block size and allocation
    uint8_t block_alloc =  
        *(uint8_t*)block_to_free & ONE_BIT_HEADER_ALLOC_MASK;

    uint8_t block_size = 
        *(uint8_t*)block_to_free >> TWO_BIT_HEADER_SIZE_SHIFT;

    //Check block to free is allocated
    if(block_alloc == UNALLOCATED) {
        perror("cannot free available block!\n");
        return 1;
    }

    //Free block and buddies of allocated block
    else{
        free_buddies(block_to_free, 
            block_size, block_alloc, heapstart, buddy_allocator);
        }
    
    //Return 0 success
    return 0;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {

    /*########################################################################

        FUNCTION: VIRTUAL REALLOC

        Given an allocated block of memory, 
        rellocated the block and its data to a new block of specified size
        If no block is provided, acts as virtual_malloc
        If no size is provided, acts as virtual_free
        If no valid block is found, allocation remains unchanged

        Input: Pointer to heapstart, Pointer to block_to_free, size for realloc

        Returns: Void pointer to reallocated block

    #########################################################################*/


    //Check that provided block pointer is within heap
    if(ptr >= virtual_sbrk(0) || ptr < heapstart) {
        return NULL;
    }

    //if no block provided, virtual_malloc given size
    if(ptr == NULL) {
        //If we have size to allocate, 
        //virtual_malloc and return pointer to block
        if(size > DEFAULT_VALUE) {
            void * new_block_data = virtual_malloc(heapstart, pow_2(size));
            void* new_block = (void*)(new_block_data)-HEADER_OFFSET;
            return (void*)new_block;
        }
        //No size to allocate
        //No memory to free
        else{
            return NULL;           
        }
    }

    //Check if block_to_reallocate is mapped to valid section of memory
    void *old_block = (void*)((char*)ptr-HEADER_OFFSET);
    uint8_t old_size = *(uint8_t*)old_block >> TWO_BIT_HEADER_SIZE_SHIFT;
    uint8_t old_alloc = *(uint8_t*)old_block & ONE_BIT_HEADER_ALLOC_MASK;
    size = log_2(size);

    //Check old block was allocated and valid
    if(old_alloc == UNALLOCATED || old_size <= DEFAULT_VALUE) {
        return NULL;
    }

    //Create new memory of desired size **AS IF MEMORY WAS ALREADY FREED!**
    //Copy reallocation block data to temporary block at end of heap
    else if (ptr != NULL) {

        //if no size provided, virtual_free old block
        if(size == UNALLOCATED) {
            virtual_free(heapstart,ptr);
            return NULL;
        }

        //Create new temp block at end of virtual heap
        uint8_t* temp_block = virtual_sbrk(1 + pow_2(old_size));

        //Copy block data from reallocation block to temp block
        memmove(temp_block, old_block, pow_2(old_size) + HEADER_OFFSET);

        //Free old block
        virtual_free(heapstart, ptr);

        //Malloc new bloc
        void * new_block_data = virtual_malloc(heapstart, pow_2(size));

        //If the new allocation fails, the original allocation must be unchanged
        
        if(new_block_data == NULL) {
            //re-malloc old memory and copy back old data
            void * reinserted_old_block_data = 
            virtual_malloc(heapstart, pow_2(old_size));

            void * reinserted_old_block = 
            (void*)(reinserted_old_block_data)-HEADER_OFFSET;

            uint8_t old_block_size = 
            *(uint8_t*)temp_block >> TWO_BIT_HEADER_SIZE_SHIFT;

            memmove(reinserted_old_block, 
            temp_block, pow_2(old_block_size)+ HEADER_OFFSET);

            //resize heap
            virtual_sbrk(-(pow_2(old_size) + HEADER_OFFSET));
            //return NULL
            return NULL;

        }else{
            //Reallocation successful
            //Copy old data to new block
            void * new_block = 
            (void*)(char*)(new_block_data)-HEADER_OFFSET;

            uint8_t new_block_size = 
            *((uint8_t*)new_block) >> TWO_BIT_HEADER_SIZE_SHIFT;

            memmove(new_block + HEADER_OFFSET, temp_block 
            + HEADER_OFFSET, pow_2(new_block_size));

            //Resize heap, removing temp block
            virtual_sbrk(-(pow_2(old_size) + HEADER_OFFSET));
            //Return reallocated block
            return (void*)(new_block) + HEADER_OFFSET;
        }
    }
    return NULL;
}

void virtual_info(void * heapstart) {

    /*########################################################################

        FUNCTION: VIRTUAL INFO

        Given a virtual heap, iterates over blocks of memory from left to right,
        printing allocation status and block size

        Input: Pointer to heapstart

        Returns: VOID

    #########################################################################*/

    //Initialize buddy allocator
    struct buddy_allocator* buddy_allocator = heapstart;

    //Set pointer to first block
    void* curr_block = (void*)&(buddy_allocator->heap_memory[0]);

    //Get first block size and allocation status
    uint8_t curr_block_size = 
        *(uint8_t*)curr_block >> TWO_BIT_HEADER_SIZE_SHIFT;

    uint8_t curr_allocated  
        = *(uint8_t*)curr_block & ONE_BIT_HEADER_ALLOC_MASK;

    //Iterate over blocks until end of heap
    while(curr_block != NULL && curr_block < virtual_sbrk(0)) {

        //Point to next block
        void * next_block = (curr_block) + pow_2(curr_block_size) 
        + (pow_2(curr_block_size) / pow_2(buddy_allocator->min_size));

        //Print allocation status and size
        if (curr_allocated == ALLOCATED) {
            printf("allocated %i\n", pow_2(curr_block_size));
            fflush(stdout);

        }else if (curr_allocated == UNALLOCATED) {
            printf("free %i\n", pow_2(curr_block_size));
            fflush(stdout);
        }
        //Set current block to nextblock
        curr_block = next_block;
        //Get size and allocation status of new current block
        curr_allocated  
            = *(uint8_t*)curr_block & ONE_BIT_HEADER_ALLOC_MASK;

        curr_block_size 
            = *(uint8_t*)curr_block >> TWO_BIT_HEADER_SIZE_SHIFT;
    }
    return;
}
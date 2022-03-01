/*############################################################################

    COMP2017 USYD 2021 SEMSTER 1
    
    ASSIGNMENT 3 : "FRIENDSHIP ENDED WITH MALLOC"

    SID: 500611960

    TESTS.C

#############################################################################*/

#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*############################################################################

    TESTS

#############################################################################*/

//Initialize static variables for virtual memory space
static uint32_t size_of_heap = 1000000000;
static uint8_t virtual_heap_space[1000000000] = {0};
static void * ptr = virtual_heap_space;

void * virtual_sbrk(int32_t increment) {

/*############################################################################

    FUCNTION: VIRTUAL SBRK

    Sets end of virtual heap in memory
    Increments virtual heap size if increment provided

    Input: increment value to inrease or decrease heap memory
            
    Return: Void * to next free memory address at end of heap (pre increment)

#############################################################################*/

    if(increment == 0){
        return (void*)(ptr+size_of_heap);}
    
    size_of_heap = size_of_heap + increment;
    return (void *)(ptr+size_of_heap);
}

/*############################################################################

    INIT TESTS

#############################################################################*/

void test_init(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_info(heapstart);
}

void test_init_init_smaller_than_min(){
    void * heapstart = ptr;
    init_allocator(heapstart,11,18);
    virtual_info(heapstart);
}

void test_init_min_zero(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,0);
    virtual_info(heapstart);    
}

void test_init_zero(){
    void * heapstart = ptr;
    init_allocator(heapstart,0,0);
    virtual_info(heapstart);   
}

/*############################################################################

    MALLOC TESTS

#############################################################################*/

void test_malloc(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 4096);
    virtual_info(heapstart);
}

void test_malloc_uneven_values(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 911);
    virtual_malloc(heapstart, 1337);
    virtual_malloc(heapstart, 8008);
    virtual_info(heapstart);
}

void test_malloc_under_min(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 1024);
    virtual_info(heapstart);
}

void test_malloc_over_initial_size(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 524288);
    virtual_info(heapstart);
}

void test_malloc_when_full(){
    void * heapstart = ptr;
    init_allocator(heapstart,10,5);
    virtual_malloc(heapstart, 1024);
    virtual_malloc(heapstart, 1024);
    virtual_info(heapstart);
}

void test_malloc_big_split(){
    void * heapstart = ptr;
    init_allocator(heapstart,20,1);
    virtual_malloc(heapstart, 1);
    virtual_info(heapstart);
}

void test_malloc_zero(){
    void * heapstart = ptr;
    init_allocator(heapstart,20,1);
    virtual_malloc(heapstart, 0);
    virtual_info(heapstart);
}

/*############################################################################

    Free TESTS

#############################################################################*/

void test_free_basic(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, A);
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_multiple(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 4096);
    void * B = virtual_malloc(heapstart, 2048);
    void * C = virtual_malloc(heapstart, 4096);
    virtual_free(heapstart, A);
    virtual_free(heapstart, B);
    int retval = virtual_free(heapstart, C);
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_invalid(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, (void*)999);
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_invalid_ptr(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, (void*)999);
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_invalid_NULL(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, NULL);
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_invalid_OOR(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, ((virtual_sbrk(0))+100000000));
    printf("%i",retval);
    virtual_info(heapstart);
}

void test_free_invalid_ZERO(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    int retval = virtual_free(heapstart, 0);
    printf("%i",retval);
    virtual_info(heapstart);
}

/*############################################################################

    REALLOC TESTS

#############################################################################*/

void test_realloc_basic(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 2048);
    virtual_realloc(heapstart,A, 4096);
    virtual_info(heapstart);
}

void test_realloc_basic_write_test(){
    void * heapstart = ptr;
    init_allocator(heapstart,8,3);
    virtual_malloc(heapstart, 8);
    void * B = virtual_malloc(heapstart, 8);
    virtual_info(heapstart);
    char test_data[8] = {'A'};
    memmove((char*)B,test_data,8);
    virtual_realloc(heapstart,B, 16);
    char test_data_retrieved[8];
    memmove(test_data_retrieved, (char*)B, 8);
    for(int i =0; i<8;i++){
        printf("%c",test_data_retrieved[i]);
    }
    virtual_info(heapstart);
}

void test_realloc_multiple(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 2048);
    void * B = virtual_malloc(heapstart, 2048);
    void * C = virtual_malloc(heapstart, 2048);
    void * D = virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 4096);
    virtual_realloc(heapstart,B, 4096);
    virtual_realloc(heapstart,C, 4096);
    virtual_realloc(heapstart,D, 4096);
    virtual_info(heapstart);
}

void test_realloc_NULL(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,NULL, 4096);
    virtual_info(heapstart);
}

void test_realloc_TOO_BIG(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    void * D = virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,D , 262144);
    virtual_info(heapstart);
}

void test_realloc_Smoller(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 1024);
    virtual_info(heapstart);
}

void test_realloc_min_to_init(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 262144);
    virtual_info(heapstart);
}

void test_realloc_ZERO_SIZE(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 0);
    virtual_info(heapstart);
}

void test_realloc_unchanged(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 4096);
    printf("%p",(void*)(A-heapstart));
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 262144);
    printf("%p",(void*)(A-heapstart));
    virtual_info(heapstart);
}

void test_realloc_past_heap(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_malloc(heapstart, 4096);
    virtual_info(heapstart);
    virtual_realloc(heapstart,(void*)9999999999999, 2048);
    virtual_info(heapstart);
}

void test_realloc_same_size(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    void * A = virtual_malloc(heapstart, 4096);
    printf("%p",(void*)(A-heapstart));
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 4096);
    printf("%p",(void*)(A-heapstart));
    virtual_info(heapstart);
}

/*############################################################################

    MULTI TESTS

#############################################################################*/

void test_all_basic_write(){
    void * heapstart = ptr;
    init_allocator(heapstart,18,11);
    virtual_info(heapstart);
    void * A = virtual_malloc(heapstart, 4096);
    void * B = virtual_malloc(heapstart, 2048);
    void * C = virtual_malloc(heapstart, 5000);
    unsigned char junkarray[5000] = {9};
    memcpy((uint8_t**)C,junkarray, 5000);
    virtual_info(heapstart);
    virtual_realloc(heapstart,A, 4096);
    virtual_info(heapstart);
    void * D = virtual_malloc(heapstart, 2048);
    virtual_info(heapstart);
    virtual_free(heapstart, C);
    virtual_info(heapstart);
    virtual_free(heapstart, B);
    virtual_info(heapstart);
    virtual_free(heapstart, D);            
    virtual_info(heapstart);
    virtual_free(heapstart, A);
    virtual_info(heapstart);
}

/*############################################################################
 
    MAIN

    Takes argv integer input and runs associated test from function
    pointer array.

#############################################################################*/

int main(int argc, char**argv) {

    void (*tests[30])();

    tests[0] = &test_init;
    tests[1] = &test_init_init_smaller_than_min;
    tests[2] = &test_init_min_zero;
    tests[3] =  &test_init_zero;
    tests[4] = &test_malloc;
    tests[5] =  &test_malloc_uneven_values;
    tests[6] =  &test_malloc_under_min;
    tests[7] =  &test_malloc_over_initial_size;
    tests[8] =  &test_malloc_when_full;
    tests[9] =  &test_malloc_big_split;
    tests[10] =  &test_malloc_zero;
    tests[11] =  &test_free_basic;
    tests[12] =  &test_free_multiple;
    tests[13] =  &test_free_invalid;
    tests[14] =  &test_free_invalid_ptr;
    tests[15] =  &test_free_invalid_NULL;
    tests[16] =  &test_free_invalid_OOR;
    tests[17] =  &test_free_invalid_ZERO;
    tests[18] =  &test_realloc_basic;
    tests[19] =  &test_realloc_basic_write_test;
    tests[20] =  &test_realloc_multiple;
    tests[21] = &test_realloc_NULL;
    tests[22] = &test_realloc_TOO_BIG;
    tests[23] =  &test_realloc_Smoller;
    tests[24] =  &test_realloc_min_to_init;
    tests[25] =  &test_realloc_ZERO_SIZE;
    tests[26] = &test_realloc_unchanged;
    tests[27] =  &test_realloc_past_heap;
    tests[28] = &test_realloc_same_size;
    tests[29] =  &test_all_basic_write;

    int test_number = atoi(argv[1]);

    (*tests[test_number])();
    
    return 0;
}

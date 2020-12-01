/* mem.h -  structs, memList, helper functions */

#ifndef MEM_H
#define MEM_H

// Memory data structure
typedef struct memHeader {
    unsigned long size;
    struct memHeader *prev;
    struct memHeader *next;
    char* sanityCheck;
    unsigned char dataStart[0];
} memHeader;

// Initial list of free memory
memHeader *memList;

// Size of memheader
#define MEM_HEADER_SIZE sizeof(memHeader)

// Min stack size
#define MIN_STACK_SIZE 4096

/**
 * @brief Finds and returns the first available memory slot
 *        Checks next ptrs, then prev ptrs.
 * 
 * @param {memSize} the memory size to check
 * @returns the memory slot or null if there is no available size
 */
extern void *findMemSlot(unsigned long memSize);

/**
 * @brief Splits the memory given the memory size.
 *        Links the new free space as well.
 * 
 * @param {memSlot} the current memory to allocate space
 * @param {memSize} the memory size to check
 * @returns memSlot
 */
extern void allocateMemSlot(memHeader *memSlot, unsigned long memSize);

/**
 * @brief Does a sanity check. ptr - MEM_HEADER_SIZE == ptr
 * 
 * @param {memSlot} The address pointing to memory slot.
 * @param {ptr} The pointer given to kfree.
 * @returns 1 if ok, 0 otherwise.
 */
extern int sanityCheck(memHeader *memSlot, void *ptr);

/**
 * @brief Searches the spot and inserts memSlot to the free list.
 *        if memList is NULL, all slots allocated, memList = memSlot
 * 
 * @param {memSlot} The current memSlot to add to the list.
 */
extern void insertToMemList(memHeader *memSlot);

/**
 * @brief Merges adjacent, if they match, memories to the current memSlot.
 * 
 * @param {memSlot} The current memSlot in the freeList to merge.
 */ 
extern void mergeMemSlot(memHeader *memSlot);

#endif
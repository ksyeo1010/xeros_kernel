/* mem.c : memory manager
 */

#include <i386.h>
#include <xeroskernel.h>
#include <mem.h>

/* variable declarations */
extern long freemem; /* free memory start */
extern char	*maxaddr; /* max addr */

////////////////////////////////////////////////////////////
void kmeminit(void) {
    memHeader *nextMemSlot = (memHeader *) HOLEEND; /* mem slot after HOLEEND */

    memList = (memHeader *) freemem;
    memList->size = HOLESTART - (unsigned long) memList->dataStart;
    memList->prev = NULL;
    memList->next = nextMemSlot;

    nextMemSlot->size = (unsigned long) maxaddr - (unsigned long) nextMemSlot->dataStart;
    nextMemSlot->size = (nextMemSlot->size/16)*16; // make to 16
    nextMemSlot->prev = memList;
    nextMemSlot->next = NULL;
}

////////////////////////////////////////////////////////////
void *kmalloc(size_t size) {
    unsigned long memSize; /* size divisible by 16 */
    memHeader *memSlot; /* memSlot to return */

    // there is no free spot
    if (memList == NULL) {
        kprintf("There is no more free space.\n");
        return NULL;
    }

    // set to minimum stack size
    if (size < MIN_STACK_SIZE) {
        memSize = MIN_STACK_SIZE;
    } else {
        memSize = (size/16) + ((size%16)?1:0);
        memSize = memSize*16;
    }

    // search for possible slots in the memory
    memSlot = findMemSlot(memSize);
    if (memSlot == NULL) {
        kprintf("Unable to allocate memory.\n");
        return NULL;
    }

    // allocate mem slot
    allocateMemSlot(memSlot, memSize);
    memSlot->sanityCheck = (char *) memSlot;
    
    return memSlot->dataStart;
}

////////////////////////////////////////////////////////////
int kfree(void *ptr) {
    unsigned long addr; /* the addr of dataStart[0] */
    memHeader *memSlot; /* the current memSlot to add back to memList */

    // check addr constraints
    addr = (unsigned long) ptr;
    if (addr < freemem) {
        kprintf("Address should not be less than free mem.\n");
        return FAILED;
    }
    if (addr > (unsigned long) maxaddr) {
        kprintf("Address should not greater than max addr.\n");
        return FAILED;
    }
    if (addr > HOLESTART && addr < HOLEEND) {
        kprintf("Address should not be in the HOLE.\n");
        return FAILED;
    }

    memSlot = (memHeader *) (addr - MEM_HEADER_SIZE);
    if (!sanityCheck(memSlot, ptr)) {
        kprintf("Something wrong occurred while doing sanity check.\n");
        return FAILED;
    }
    insertToMemList(memSlot);
    mergeMemSlot(memSlot);
    return SUCCEED;
}
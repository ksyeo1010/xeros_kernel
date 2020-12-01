/* memHeader.c */

#include <xeroskernel.h>

/* Helper function decalrations */

void reallocateMemPtr(void); /* sets list back to head */

////////////////////////////////////////////////////////////
void *findMemSlot(unsigned long memSize) {
    memHeader *memSlot = memList;
    // search from curr mem slot going right along the list
    do {
        if (memSlot->size >= memSize) {
            return memSlot;
        }
    } while ((memSlot = memSlot->next) != NULL);
    return NULL;
}

////////////////////////////////////////////////////////////
void allocateMemSlot(memHeader *memSlot, unsigned long memSize) {
    // if memslot->size == memSize just hop to next
    if (memSlot->size == memSize) {
        // merge prev and next
        if (memSlot->prev != NULL) {
            memSlot->prev->next = memSlot->next;
        }
        if (memSlot->next != NULL) {
            memSlot->next->prev = memSlot->prev;
        }
        memList = memSlot->next;
    } else {
        // current mem = address of memSlot + size of data + size of header
        memList = (memHeader *) ((unsigned long) memSlot + memSize + MEM_HEADER_SIZE);
        // curremt mem size = current mem size - the size to allocate - size of header
        memList->size = memSlot->size - memSize - MEM_HEADER_SIZE;
        memList->next = memSlot->next;
        memList->prev = memSlot->prev;

        // reorganize next and prev ptrs
        if (memList->next != NULL) {
            memList->next->prev = memList;
        }
        if (memList->prev != NULL) {
            memList->prev->next = memList;
        }
    }

    memSlot->size = memSize;
    memSlot->next = NULL;
    memSlot->prev = NULL;

    // set memList to head 
    reallocateMemPtr();
}

////////////////////////////////////////////////////////////
int sanityCheck(memHeader *memSlot, void *ptr) {
    if ((unsigned long) memSlot->sanityCheck == ((unsigned long) ptr - MEM_HEADER_SIZE)) {
        return SUCCEED;
    }
    return FAILED;
}

////////////////////////////////////////////////////////////
void insertToMemList(memHeader *memSlot) {
    memSlot->sanityCheck = NULLCH;
    // if there is nothing in free list just return current slot
    if (memList == NULL) {
        memList = memSlot;
        return;
    }

    for (;;) {
        /* if memslot < memList */
        if (memList->prev == NULL && memSlot < memList) {
            memSlot->prev = NULL;
            memSlot->next = memList;
            memList->prev = memSlot;
            break;
        }
        /* if next mem is NULL we know its greatest */
        if (memList->next == NULL) { 
            memList->next = memSlot;
            memSlot->prev = memList;
            memSlot->next = NULL;
            break;
        }
        /* if in between */
        if (memList < memSlot && memList->next > memSlot) {
            memSlot->next = memList->next;
            memSlot->prev = memList;
            memList->next->prev = memSlot;
            memList->next = memSlot;
            break;
        }
        /* else keep on going */
        memList = memList->next;
    }

    // set memList to head 
    reallocateMemPtr();
}

////////////////////////////////////////////////////////////
void mergeMemSlot(memHeader *memSlot) {
    // merge prev with memSlot if prev+size == memSlot
    if (memSlot->prev != NULL &&
        (unsigned long) memSlot == 
        ((unsigned long) memSlot->prev + memSlot->prev->size + MEM_HEADER_SIZE)) {
        memSlot->prev->size += MEM_HEADER_SIZE + memSlot->size;
        memSlot->prev->next = memSlot->next;
        if (memSlot->next != NULL) {
            memSlot->next->prev = memSlot->prev;
        }
        memSlot = memSlot->prev; /* set memslot to the combined mem block */
    }
    // merge right with memSlot if memSlot+size == next
    if (memSlot->next != NULL && 
        ((unsigned long) memSlot + memSlot->size + MEM_HEADER_SIZE) 
        == (unsigned long) memSlot->next) {
        memSlot->size += MEM_HEADER_SIZE + memSlot->next->size;
        memSlot->next = memSlot->next->next;
        if (memSlot->next != NULL) {
            memSlot->next->prev = memSlot;
        }
    }
}

/**
 * @brief Moves the memList ptr all the way to the start.
 */ 
void reallocateMemPtr() {
    if (memList == NULL) return; /* in case there is no "mem" to reallocate */
    while (memList->prev != NULL) {
        memList = memList->prev;
    }
}
/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{
    kprintf( "\n\nCPSC 415, 2020W1\n32 Bit Xeros -20.9.9 - Closer to beta \nLocated at: %x to %x\n", 
        &entry, &end); 

    // init memory
    kmeminit();
    // init dispatcher
    dispInit();
    // init context switcher
    contextinit();
    // init semaphore
    seminit();
    // init devices
    devinit();
    // create idleprocess, first process every created
    create(idleproc, STACKSIZE);
    // init root function
    create(root, STACKSIZE);
    // dispatcher
    dispatch();
    
    kprintf("\n\nWhen your  kernel is working properly ");
    kprintf("this line should never be printed!\n");
    for(;;) ; /* loop forever */
}



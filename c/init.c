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
  
    char str[1024];
    int a = sizeof(str);
    int b = -17;
    int i; 

    kprintf( "\n\nCPSC 415, 2020W1\n32 Bit Xeros -20.9.9 - Closer to beta \nLocated at: %x to %x\n", 
        &entry, &end); 

    kprintf("Some sample output to illustrate different types of printing\n\n");

    /* A busy wait to pause things on the screen, Change the value used 
        in the termination condition to control the pause
    */

    for (i = 0; i < 3000000; i++);

    /* Build a string to print) */
    sprintf(str, 
        "This is the number -17 when printed signed %d unsigned %u hex %x and a string %s.\n      Sample printing of 1024 in signed %d, unsigned %u and hex %x.",
        b, b, b, "Hello", a, a, a);

    /* Print the string */

    kprintf("\n\nThe %dstring is: \"%s\"\n\nThe formula is %d + %d = %d.\n\n\n", 
        a, str, a, b, a + b);

    for (i = 0; i < 4000000; i++);
    /* or just on its own */
    kprintf(str);

    /* Add your code below this line and before next comment */
    kprintf("\n\n");

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
    
#ifdef TEST
    // run test
    run_test();
#else
    // init root function
    create(root, STACKSIZE);
#endif

    // dispatcher
    dispatch();
    

    for (i = 0; i < 2000000; i++);
    /* Add all of your code before this comment and after the previous comment */
    /* This code should never be reached after you are done */
    kprintf("\n\nWhen your  kernel is working properly ");
    kprintf("this line should never be printed!\n");
    for(;;) ; /* loop forever */
}



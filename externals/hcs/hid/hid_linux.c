#ifdef __linux__


#include <linux/input.h>
#include <sys/ioctl.h>


#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)



/* The application reading the device is supposed to queue all events up to */
/* the SYN_REPORT event, and then process them, so that a mouse pointer */
/* will move diagonally instead of following the sides of a rectangle, */
/* which would be very annoying. */


#endif  /* #ifdef __linux__ */



#include <m_pd.h>

#include "input_arrays.h"

static char *version = "$Revision: 1.1 $";

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *hid_class;

typedef struct _hid 
{
		t_object            x_obj;
		t_int               x_fd;
		t_symbol            *x_devname;
		t_clock             *x_clock;
		t_int               x_read_ok;
		t_int               x_started;
		t_int               x_delay;
		t_int               x_vendorID;
		t_int               x_productID;
		t_int               x_locID;
} t_hid;


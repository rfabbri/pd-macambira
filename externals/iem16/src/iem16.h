/* ********************************************** */
/* the IEM16 external                              */
/* ********************************************** */
/*                            forum::für::umläute */
/* ********************************************** */

/* the IEM16 external is a runtime-library for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the IEM16 external
 * (except if you want to use the code for other things)
 * download "pure data" at

 http://pd.iem.at
 ftp://iem.at/pd

 *
 * if you are looking for the latest release of the IEM16-external you should have another look at

 ftp://iem.at/pd/Externals/IEM16

 * 
 * IEM16 is published under the GNU GeneralPublicLicense, that must be shipped with IEM16.
 * if you are using Debian GNU/linux, the GNU-GPL can be found under /usr/share/common-licenses/GPL
 * if you still haven't found a copy of the GNU-GPL, have a look at http://www.gnu.org
 *
 * "pure data" has it's own license, that comes shipped with "pure data".
 *
 * there are ABSOLUTELY NO WARRANTIES for anything
 */

#ifndef INCLUDE_IEM16_H__
#define INCLUDE_IEM16_H__

#include "m_pd.h"

typedef short t_iem16_16bit;

#define IEM16_SCALE_UP (32767)
#define IEM16_SCALE_DOWN (1./32767)

#define VERSION "0.1"

#endif

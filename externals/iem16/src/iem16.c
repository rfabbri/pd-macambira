/* ...this is a very IEM16 external ...
   it allows for 16bit-constructs where float would eat too much memory
	
   forum::für::umläute@IEM:2003
*/

#include "iem16.h"

/* do a little help thing */

typedef struct iem16 {
  t_object t_ob;
} t_iem16;

t_class *iem16_class;

void *iem16_new(void){
  t_iem16 *x = (t_iem16 *)pd_new(iem16_class);
  post("iem16: 16bit objects for low memory usage");
  return (void *)x;
}

/* include some externals */
void iem16_table_setup();
void iem16_array_setup();
void iem16_array_tilde_setup();
void iem16_delay_setup();

void iem16_setup(void) {
  iem16_table_setup();
  iem16_array_setup();
  iem16_array_tilde_setup();
  iem16_delay_setup();

  /* ************************************** */
  post("iem16:\t16bit-objects for low memory usage");
  post("iem16:\t(l) forum::für::umläute\t\tIOhannes m zmölnig");
  post("iem16:\tInstitute of Electronic Music and Acoustics, Graz - iem");
  post("iem16:\tcompiled: "__DATE__);
  

  iem16_class = class_new(gensym("iem16"), 
			  iem16_new, 
			  0, 
			  sizeof(t_iem16), CLASS_NOINLET, A_NULL);
  class_addcreator((t_newmethod)iem16_new, 
		   gensym("IEM16"), A_NULL); 
}

void IEM16_setup(void){
  iem16_setup();
}

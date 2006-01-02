/*#include <m_pd.h>*/
#include "../../pd/src/m_pd.h"
void pureunity_setup() {
  t_pd *m = &pd_objectmaker;
  class_addcreator((t_newmethod)getfn(m,gensym( "inlet" )),gensym("f.inlet" ),A_GIMME,0);
  class_addcreator((t_newmethod)getfn(m,gensym( "inlet~")),gensym("~.inlet" ),A_GIMME,0);
  class_addcreator((t_newmethod)getfn(m,gensym("outlet" )),gensym("f.outlet"),A_GIMME,0);
  class_addcreator((t_newmethod)getfn(m,gensym("outlet~")),gensym("~.outlet"),A_GIMME,0);
}

/* glue code for compiling "template" as a library
 * this calls the setup function of each object
 */
void mycobject_setup(void);

void template_setup(void) {
  mycobject_setup();
}

#include "../cxc/mean.c"
void mean_setup()
{
  cxmean_setup();
  cxavgdev_setup();
  cxstddev_setup();
  mean_tilde_setup();
}

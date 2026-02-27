#define EMM_STRIP_PREFIXES
#include <emm.h>

#include <stdio.h>

int main(void) {
  vec3 v = {0.1f, 0.2f, 0.3f};
  (void)v;

  printf("v: %f, %f, %f\n", v.r, v.y, v.raw[2]);

  return 0;
}

#ifndef EMM_H_
#define EMM_H_

#include <assert.h>
#include <math.h>

#ifdef __cplusplus
  #define HEADER_BEGIN extern "C" {
  #define HEADER_END }
#else
  #define HEADER_BEGIN
  #define HEADER_END
#endif

HEADER_BEGIN

typedef union vec2_t {
  struct { float x, y; };
  struct { float r, g; };
  float raw[2];
} vec2;

typedef union vec3_t {
  struct { float x, y, z; };
  struct { float r, g, b; };
  float raw[3];
} vec3;

typedef union vec4_t {
  struct { float x, y, z, w; };
  struct { float r, g, b, a; };
  float raw[4];
} vec4;

typedef union mat2_t {
  float raw[4];
  vec2 columns[2];
} mat2;

typedef union mat3_t {
  float raw[9];
  vec3 columns[3];
} mat3;

typedef union mat4_t {
  float raw[16];
  vec4 columns[4];
} mat4;

HEADER_END

#endif // EMM_H_

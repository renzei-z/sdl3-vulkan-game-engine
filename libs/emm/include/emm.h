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

typedef union emm_vec2_t {
  struct { float x, y; };
  struct { float r, g; };
  float raw[2];
} emm_vec2;

typedef union emm_vec3_t {
  struct { float x, y, z; };
  struct { float r, g, b; };
  float raw[3];
} emm_vec3;

typedef union emm_vec4_t {
  struct { float x, y, z, w; };
  struct { float r, g, b, a; };
  float raw[4];
} emm_vec4;

typedef union emm_mat2_t {
  float raw[4];
  emm_vec2 columns[2];
} emm_mat2;

typedef union emm_mat3_t {
  float raw[9];
  emm_vec3 columns[3];
} emm_mat3;

typedef union emm_mat4_t {
  float raw[16];
  emm_vec4 columns[4];
} emm_mat4;

#ifdef EMM_STRIP_PREFIXES

typedef emm_vec2 vec2;
typedef emm_vec3 vec3;
typedef emm_vec4 vec4;

typedef emm_mat2 mat2;
typedef emm_mat3 mat3;
typedef emm_mat4 mat4;

#endif // EMM_NO_STRIP_PREFIXES

HEADER_END

#endif // EMM_H_

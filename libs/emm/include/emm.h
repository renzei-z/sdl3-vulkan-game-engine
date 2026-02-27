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
  // _unused_w/a is included to ensure that emm_vec3 is aligned to 16-bytes.
  struct { float x, y, z, _unused_w; };
  struct { float r, g, b, _unused_a; };
  float raw[4];
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

// NOTE: We use 12 elements for raw here as, because of padding, the structure
// in memory is:
// [0] [1] [2] [PAD]
// [3] [4] [5] [PAD]
// [6] [7] [8] [PAD]
// and only using 9 would cut off after [6].

typedef union emm_mat3_t {
  float raw[12];
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

#ifdef EMM_ASSERT_ALIGNMENTS
#define emm_check_align(type, size, name) static_assert(sizeof(type) == size, "Alignment error: "name". This is being treated as an error, as EMM_ASSERT_ALIGNMENTS is defined.")

emm_check_align(emm_vec2,  8, "emm_vec2");
emm_check_align(emm_vec3, 16, "emm_vec3");
emm_check_align(emm_vec4, 16, "emm_vec4");

emm_check_align(emm_mat2, 16, "emm_mat2");
emm_check_align(emm_mat3, 48, "emm_mat3");
emm_check_align(emm_mat4, 64, "emm_mat4");

#endif // EMM_ASSERT_ALIGNMENTS

HEADER_END

#endif // EMM_H_

#ifndef MATH_TYPES_H_
#define MATH_TYPES_H_

#include <core/cpp_header_guard.h>

HEADER_BEGIN

#include <math.h>

// Vectors

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

// Matrices

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

static inline vec2 v2_add(vec2 a, vec2 b) {
  return (vec2) { a.x + b.x, a.y + b.y };
}

static inline vec2 v2_sub(vec2 a, vec2 b) {
  return (vec2) { a.x - b.x, a.y - b.y };
}

static inline vec2 v2_muls(vec2 a, float s) {
  return (vec2) { a.x * s, a.y * s };
}

static inline float v2_len2(vec2 a) {
  return a.x * a.x + a.y * a.y;
}

static inline float v2_len(vec2 a) {
  return sqrtf(v2_len2(a));
}

static inline vec2 v2_norm(vec2 a) {
  float l = v2_len(a);
  if (l == 0) return (vec2){0};
  return v2_muls(a, 1.0f / l);
}

static inline float v2_dot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}

static inline float v2_cross(vec2 a, vec2 b) {
  return a.x * b.y - a.y * b.x;
}

static inline vec3 v3_add(vec3 a, vec3 b) {
  return (vec3) { a.x + b.x, a.y + b.y, a.z + b.z};
}

// TODO: vec3 basic inline operations

static inline vec4 v4_add(vec4 a, vec4 b) {
  return (vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

// TODO: vec4 basic inline operations



HEADER_END

#endif // MATH_TYPES_H_

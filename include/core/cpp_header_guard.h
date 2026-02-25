#ifndef CPP_HEADER_GUARD_H_
#define CPP_HEADER_GUARD_H_

#ifdef __cplusplus
  #define HEADER_BEGIN extern "C" {
  #define HEADER_END }
#else
  #define HEADER_BEGIN
  #define HEADER_END
#endif

#endif // CPP_HEADER_GUARD_H_

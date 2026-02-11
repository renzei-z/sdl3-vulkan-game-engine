#ifndef UTIL_H_
#define UTIL_H_

#define free_and_set_null(x) { \
    free(x);                   \
    x = NULL;                  \
} 

#endif // UTIL_H_
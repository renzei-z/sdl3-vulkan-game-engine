#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <core/cpp_header_guard.h>

HEADER_BEGIN

#include <stdbool.h>

typedef enum engine_event_type_t {
    ENGINE_EVENT_NONE,
    ENGINE_EVENT_QUIT,
    ENGINE_EVENT_RESIZE
} engine_event_type;

typedef struct engine_event_t {
    engine_event_type type;
} engine_event;

bool platform_poll_events(engine_event *out);

HEADER_END
  
#endif // PLATFORM_H_

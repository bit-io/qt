#include "common.hpp"

#include <cstdlib>

using namespace qth;
using namespace qth::api;

extern "C" {

int32_t     qth_poll_event(void)    { return events().poll(); }
int64_t     qth_event_handle(void)  { return events().current().handle; }
int32_t     qth_event_value(void)   { return events().current().value; }
const char* qth_event_text(void)    { return out(events().current().text); }
int32_t     qth_event_x(void)       { return events().current().x; }
int32_t     qth_event_y(void)       { return events().current().y; }
int32_t     qth_event_mods(void)    { return events().current().mods; }
int32_t     qth_event_pending(void) { return static_cast<int32_t>(events().pending()); }
void        qth_event_clear(void)   { events().clear(); }

void qth_free_string(const char* s) { std::free(const_cast<char*>(s)); }

} // extern "C"

#include "types.h"
#ifndef TERMINAL_H
#define TERMINAL_H

void enable_raw_mode(struct cursor_state*);
void disable_raw_mode(void);

#endif

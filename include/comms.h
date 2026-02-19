#pragma once

#include "types.h"

void comms_read_cmd(FltStates_t *state);
void comms_send_telem(FltStates_t state, FltData_t *fltdata);
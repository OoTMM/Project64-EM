#pragma once

typedef enum {
    STATE_STARTED, // Initial evaluation and execution
    STATE_RUNNING, // Event loop running with pending events
    STATE_STOPPED,  // No pending events
    STATE_INVALID
} INSTANCE_STATE;

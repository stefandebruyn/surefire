#include "pal/Thread.hpp"

const I32 Thread::FAIR_MIN_PRI = 0;

const I32 Thread::FAIR_MAX_PRI = 0;

// Priority just above RCU kernel thread, which has priority 1 on NILRT.
const I32 Thread::REALTIME_MIN_PRI = 2;

// Priority just below the software and hardware IRQ kernel threads, which have
// priorities 14 and 15, respectively, on NILRT.
const I32 Thread::REALTIME_MAX_PRI = 13;

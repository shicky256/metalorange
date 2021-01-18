#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG (1)
#define DBG_PRINTF(fmt, ...) \
	do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)
#endif

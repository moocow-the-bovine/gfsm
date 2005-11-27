#ifndef _GFSM_DEBUG_H
#define _GFSM_DEBUG_H

#include <glib.h>

/* Define these to enable verbose memory debugging */
/*#define GFSM_MEM_DEBUG*/
/*#define GFSM_ALLOC_DEBUG*/

void gfsm_debug_init(void);
void gfsm_debug_finish(void);
void gfsm_debug_print(void);

#endif /* _GFSM_DEBUG_H */

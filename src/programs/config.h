#include <gfsmConfig.h>

/* Define this for verbose memory debugging */
//#define GFSM_MEM_DEBUG

#ifdef GFSM_MEM_DEBUG
# include <glib.h>
# define GFSM_INIT   g_mem_set_vtable(glib_mem_profiler_table);
# define GFSM_FINISH g_blow_chunks(); g_mem_profile();
#else
# define GFSM_INIT
# define GFSM_FINISH
#endif

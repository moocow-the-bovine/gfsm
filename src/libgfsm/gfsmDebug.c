#include <gfsmDebug.h>
#include <gfsmConfig.h>

#ifdef GFSM_DEBUG_ENABLED
# define GFSM_MEM_DEBUG
# define GFSM_ALLOC_DEBUG
#endif /* GFSM_DEBUG_ENABLED */

GAllocator *gfsm_node_debug_allocator=NULL;
GAllocator *gfsm_slist_debug_allocator=NULL;
GAllocator *gfsm_list_debug_allocator=NULL;

void gfsm_debug_init(void) {
#if defined(GFSM_MEM_DEBUG)
  g_mem_set_vtable(glib_mem_profiler_table);
# if defined(GFSM_ALLOC_DEBUG)
  gfsm_node_debug_allocator = g_allocator_new("gfsmNodeDebugAllocator",128);
  gfsm_slist_debug_allocator = g_allocator_new("gfsmSListDebugAllocator",128);
  gfsm_list_debug_allocator = g_allocator_new("gfsmListDebugAllocator",128);
  g_node_push_allocator(gfsm_node_debug_allocator);
  g_slist_push_allocator(gfsm_slist_debug_allocator);
  g_list_push_allocator(gfsm_list_debug_allocator);
# endif /* GFSM_ALLOC_DEBUG */
#endif /* GFSM_MEM_DEBUG */
  return;
}

void gfsm_debug_finish(void) {
#if defined(GFSM_MEM_DEBUG)
  g_blow_chunks();
# if defined(GFSM_ALLOC_DEBUG)
  g_node_pop_allocator();
  g_slist_pop_allocator();
  g_list_pop_allocator();
  g_allocator_free(gfsm_slist_debug_allocator);
  g_allocator_free(gfsm_list_debug_allocator);
  g_allocator_free(gfsm_node_debug_allocator);
  gfsm_node_debug_allocator=NULL;
  gfsm_slist_debug_allocator=NULL;
  gfsm_list_debug_allocator=NULL;
# endif /* GFSM_ALLOC_DEBUG */
#endif /* GFSM_MEM_DEBUG */
  return;
}

void gfsm_debug_print(void) {
#if defined(GFSM_MEM_DEBUG)
  g_mem_profile();
#endif /* GFSM_MEM_DEBUG */
  return;
}

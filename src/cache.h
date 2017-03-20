#ifndef CACHE_H
#define CACHE_H

#include "types.h"

extern uns64 cycle;

/////////////////////////////////////////////////////////////////////////////////////////////
// Define the Data Structures here with correct field (Look at Appendix A for more details)
//////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Cache_Line_Struct
{ bool Valid;
  bool Dirty;
  uns64 tag;
  uns core_id;
  uns32 last_access_time;
  Addr address;
}Cache_Line;

typedef struct Cache_Set_Struct
{ Cache_Line* cache_line;
}Cache_Set;

typedef struct Cache_Struct
{ uns64 num_way;
  uns32 num_set;
  Cache_Set* cache_set;
  uns64 repl_policy;
  Cache_Line last_evicted_line;
  uns64 stat_read_access;
  uns64 stat_write_access;
  uns64 stat_read_miss;
  uns64 stat_write_miss;
  uns64 stat_dirty_evicts;
}Cache;



//////////////////////////////////////////////////////////////////////////////////////////////
// Following functions might be helpful(Look at Appendix A for more details)
//////////////////////////////////////////////////////////////////////////////////////////////


Cache  *cache_new(uns64 size, uns64 assocs, uns64 linesize, uns64 repl_policy);
Flag    cache_access         (Cache *c, Addr lineaddr, uns is_write, uns core_id);
void    cache_install        (Cache *c, Addr lineaddr, uns is_write, uns core_id);
void    cache_print_stats    (Cache *c, char *header);
uns     cache_find_victim    (Cache *c, uns set_index, uns core_id);

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

#endif // CACHE_H

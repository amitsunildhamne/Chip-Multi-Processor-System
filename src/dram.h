#ifndef DRAM_H
#define DRAM_H

#include "types.h"

extern Addr full_addr;
extern Flag DRAM_PAGE_POLICY;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

typedef struct row_buffer_struct
{ Flag valid;
  uns64 Row_Id;
}Row_Buffer;

typedef struct DRAM_struct
{ Row_Buffer Row_Buffer_Entry[256];
  uns64 stat_read_access;
  uns64 stat_write_access;
  uns64 stat_read_miss;
  uns64 stat_write_miss;
  uns64 stat_dirty_evicts;
  uns64 stat_read_delay;
  uns64 stat_write_delay;
}DRAM;

DRAM   *dram_new();
void    dram_print_stats(DRAM *dram);
uns64   dram_access(DRAM *dram,Addr lineaddr, Flag is_dram_write);
uns64   dram_access_mode_CDE(DRAM *dram,Addr lineaddr, Flag is_dram_write);

#endif // DRAM_H

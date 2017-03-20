#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"

#include "memsys.h"

#include "cache.h"
#include "dram.h"

#define PAGE_SIZE 4096

//---- Cache Latencies  ------

#define DCACHE_HIT_LATENCY   1
#define ICACHE_HIT_LATENCY   1
#define L2CACHE_HIT_LATENCY  10

extern MODE   SIM_MODE;
extern uns64  CACHE_LINESIZE;
extern uns64  REPL_POLICY;

Addr full_addr;

extern uns64  DCACHE_SIZE; 
extern uns64  DCACHE_ASSOC; 
extern uns64  ICACHE_SIZE; 
extern uns64  ICACHE_ASSOC; 
extern uns64  L2CACHE_SIZE; 
extern uns64  L2CACHE_ASSOC;
extern uns64  L2CACHE_REPL;
extern uns64  NUM_CORES;
extern uns64 	cycle;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


Memsys *memsys_new(void) 
{
  Memsys *sys = (Memsys *) calloc (1, sizeof (Memsys));

  if(SIM_MODE==SIM_MODE_A){
    sys->dcache = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
  }

  if(SIM_MODE==SIM_MODE_B){
    sys->dcache = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->icache = cache_new(ICACHE_SIZE, ICACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->l2cache = cache_new(L2CACHE_SIZE, L2CACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->dram    = dram_new();
  }

  if(SIM_MODE==SIM_MODE_C){
    sys->dcache = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->icache = cache_new(ICACHE_SIZE, ICACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->l2cache = cache_new(L2CACHE_SIZE, L2CACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    sys->dram    = dram_new();
  }

  if( (SIM_MODE==SIM_MODE_D) || (SIM_MODE==SIM_MODE_E)) {
    sys->l2cache = cache_new(L2CACHE_SIZE, L2CACHE_ASSOC, CACHE_LINESIZE, L2CACHE_REPL);
    sys->dram    = dram_new();
    uns ii;
    for(ii=0; ii<NUM_CORES; ii++){
      sys->dcache_coreid[ii] = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
      sys->icache_coreid[ii] = cache_new(ICACHE_SIZE, ICACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
    }
  }

  return sys;
}


////////////////////////////////////////////////////////////////////
// This function takes an ifetch/ldst access and returns the delay
////////////////////////////////////////////////////////////////////

uns64 memsys_access(Memsys *sys, Addr addr, Access_Type type, uns core_id)
{
  uns delay=0;

  full_addr = addr;
  // all cache transactions happen at line granularity, so get lineaddr
  Addr lineaddr=addr/CACHE_LINESIZE;

  if(SIM_MODE==SIM_MODE_A){
    delay = memsys_access_modeA(sys,lineaddr,type, core_id);
  }

  if((SIM_MODE==SIM_MODE_B)||(SIM_MODE==SIM_MODE_C)){
    delay = memsys_access_modeBC(sys,lineaddr,type, core_id);
  }

  if((SIM_MODE==SIM_MODE_D)||(SIM_MODE==SIM_MODE_E)){
    delay = memsys_access_modeDE(sys,lineaddr,type, core_id);
  }
  
  //update the stats
  if(type==ACCESS_TYPE_IFETCH){
    sys->stat_ifetch_access++;
    sys->stat_ifetch_delay+=delay;
  }

  if(type==ACCESS_TYPE_LOAD){
    sys->stat_load_access++;
    sys->stat_load_delay+=delay;
  }

  if(type==ACCESS_TYPE_STORE){
    sys->stat_store_access++;
    sys->stat_store_delay+=delay;
  }


  return delay;
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void memsys_print_stats(Memsys *sys)
{
  char header[256];
  sprintf(header, "MEMSYS");

  double ifetch_delay_avg=0;
  double load_delay_avg=0;
  double store_delay_avg=0;

  if(sys->stat_ifetch_access){
    ifetch_delay_avg = (double)(sys->stat_ifetch_delay)/(double)(sys->stat_ifetch_access);
  }

  if(sys->stat_load_access){
    load_delay_avg = (double)(sys->stat_load_delay)/(double)(sys->stat_load_access);
  }

  if(sys->stat_store_access){
    store_delay_avg = (double)(sys->stat_store_delay)/(double)(sys->stat_store_access);
  }


  printf("\n");
  printf("\n%s_IFETCH_ACCESS  \t\t : %10llu",  header, sys->stat_ifetch_access);
  printf("\n%s_LOAD_ACCESS    \t\t : %10llu",  header, sys->stat_load_access);
  printf("\n%s_STORE_ACCESS   \t\t : %10llu",  header, sys->stat_store_access);
  printf("\n%s_IFETCH_AVGDELAY\t\t : %10.3f",  header, ifetch_delay_avg);
  printf("\n%s_LOAD_AVGDELAY  \t\t : %10.3f",  header, load_delay_avg);
  printf("\n%s_STORE_AVGDELAY \t\t : %10.3f",  header, store_delay_avg);
  printf("\n");

   if(SIM_MODE==SIM_MODE_A){
    sprintf(header, "DCACHE");
    cache_print_stats(sys->dcache, header);
  }
  
  if((SIM_MODE==SIM_MODE_B)||(SIM_MODE==SIM_MODE_C)){
    sprintf(header, "ICACHE");
	cache_print_stats(sys->icache, header);
	sprintf(header, "DCACHE");
    cache_print_stats(sys->dcache, header);
	sprintf(header, "L2CACHE");
    cache_print_stats(sys->l2cache, header);
    dram_print_stats(sys->dram);
  }

  if((SIM_MODE==SIM_MODE_D)||(SIM_MODE==SIM_MODE_E)){
    assert(NUM_CORES==2); //Hardcoded
	sprintf(header, "ICACHE_0");
    cache_print_stats(sys->icache_coreid[0], header);
	sprintf(header, "DCACHE_0");
    cache_print_stats(sys->dcache_coreid[0], header);
	sprintf(header, "ICACHE_1");
    cache_print_stats(sys->icache_coreid[1], header);
	sprintf(header, "DCACHE_1");
    cache_print_stats(sys->dcache_coreid[1], header);
	sprintf(header, "L2CACHE");
    cache_print_stats(sys->l2cache, header);
    dram_print_stats(sys->dram);
    
  }

}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

uns64 memsys_access_modeA(Memsys *sys, Addr lineaddr, Access_Type type, uns core_id){
 Flag needs_dcache_access=FALSE;
  Flag is_write=FALSE;
  
  if(type == ACCESS_TYPE_IFETCH){
    // no icache in this mode
  }
    
  if(type == ACCESS_TYPE_LOAD){
    needs_dcache_access=TRUE;
    is_write=FALSE;
  }
  
  if(type == ACCESS_TYPE_STORE){
    needs_dcache_access=TRUE;
    is_write=TRUE;
  }

  if(needs_dcache_access){
    Flag outcome=cache_access(sys->dcache, lineaddr, is_write, core_id);
    if(outcome==MISS){
      cache_install(sys->dcache, lineaddr, is_write, core_id);
    }
  }

  // timing is not simulated in Part A
  return 0;
}


uns64 memsys_access_modeBC(Memsys *sys, Addr lineaddr, Access_Type type,uns core_id){
uns64 delay=ICACHE_HIT_LATENCY;
Flag p;
  if(type == ACCESS_TYPE_IFETCH){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
    
    p = cache_access(sys->icache,lineaddr,false,core_id);
    if(p != HIT) 
     {
      delay = delay + memsys_L2_access(sys,lineaddr,false,core_id);
      cache_install(sys->icache,lineaddr,false,core_id);
     }
  }
    
  if(type == ACCESS_TYPE_LOAD){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
   
    delay = DCACHE_HIT_LATENCY;
    p = cache_access(sys->dcache,lineaddr,false,core_id);
    if(p != HIT) 
     {
      delay = delay + memsys_L2_access(sys,lineaddr,false,core_id);
      cache_install(sys->dcache,lineaddr,false,core_id); //install new line in l1
      //install evicted line in l2
      if (sys->dcache->last_evicted_line.Dirty==true && sys->dcache->last_evicted_line.Valid==true)
        {
         (void) memsys_L2_access(sys,sys->dcache->last_evicted_line.address,true,core_id);
        
        }
     
     }
  }
  

  if(type == ACCESS_TYPE_STORE){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
    
    delay = DCACHE_HIT_LATENCY;
    p = cache_access(sys->dcache,lineaddr,true,core_id); 
     if(p!=HIT) 
     {delay = delay + memsys_L2_access(sys,lineaddr,false,core_id);
      cache_install(sys->dcache,lineaddr,true,core_id);   //install line in l1
      //install evicted line in l2
      if (sys->dcache->last_evicted_line.Dirty==true && sys->dcache->last_evicted_line.Valid==true)
        {
         
 	 (void) memsys_L2_access(sys,sys->dcache->last_evicted_line.address,true,core_id);
      
        }
     
     }
  }
  return delay;
}

 uns64  memsys_L2_access(Memsys *sys, Addr lineaddr, Flag is_writeback, uns core_id)
{
  uns64 delay = L2CACHE_HIT_LATENCY;

  //To get the delay of L2 MISS, you must use the dram_access() function
  //To perform writebacks to memory, you must use the dram_access() function
  //This will help us track your memory reads and memory writes

  Flag temp;
  temp = cache_access(sys->l2cache,lineaddr,is_writeback,core_id);
  if(temp==MISS)
   { delay = delay + dram_access(sys->dram,lineaddr,false);
    
     cache_install(sys->l2cache,lineaddr,is_writeback,core_id);
     if (sys->l2cache->last_evicted_line.Dirty==true && sys->l2cache->last_evicted_line.Valid==true)
     { (void)dram_access(sys->dram,sys->l2cache->last_evicted_line.address,true);
     }
   }
  return delay;
}

/////////////////////////////////////////////////////////////////////
// This function converts virtual page number (VPN) to physical frame
// number (PFN).  Note, you will need additional operations to obtain
// VPN from lineaddr and to get physical lineaddr using PFN. 
/////////////////////////////////////////////////////////////////////

uns64 memsys_convert_vpn_to_pfn(Memsys *sys, uns64 vpn, uns core_id){
  uns64 tail = vpn & 0x000fffff;
  uns64 head = vpn >> 20;
  uns64 pfn  = tail + (core_id << 21) + (head << 21);
  assert(NUM_CORES==2);
  return pfn;
}

////////////////////////////////////////////////////////////////////
// --------------- DO NOT CHANGE THE CODE ABOVE THIS LINE ----------
////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////
// For Mode D/E you will use per-core ICACHE and DCACHE
// ----- YOU NEED TO WRITE THIS FUNCTION AND UPDATE DELAY ----------
/////////////////////////////////////////////////////////////////////



uns64 memsys_access_modeDE(Memsys *sys, Addr v_lineaddr, Access_Type type,uns core_id){
  uns64 delay=0;
  Addr p_lineaddr=0;
 // printf("I've entered mode DE");
  p_lineaddr = v_lineaddr;
  
  // TODO: First convert lineaddr from virtual (v) to physical (p) using the
  // function memsys_convert_vpn_to_pfn. Page size is defined to be 4KB.
  // NOTE: VPN_to_PFN operates at page granularity and returns page addr
  
  uns64 vpn = v_lineaddr>>6;
  uns64 ppn = memsys_convert_vpn_to_pfn(sys,vpn,core_id);
  uns64 index = v_lineaddr%((uns)pow(2,6)); 
  Flag p;
  Addr lineaddr=(ppn<<6)|index;
  //printf("I've converted to ");
  if(type == ACCESS_TYPE_IFETCH){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
   delay = delay+ICACHE_HIT_LATENCY;
   p = cache_access(sys->icache_coreid[core_id],lineaddr,false,core_id);
    if(p != HIT) 
     {
      delay = delay + memsys_L2_access_multicore(sys,lineaddr,false,core_id);
      cache_install(sys->icache_coreid[core_id],lineaddr,false,core_id);    
  }
}

  if(type == ACCESS_TYPE_LOAD){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
 delay = delay +DCACHE_HIT_LATENCY;
    p = cache_access(sys->dcache_coreid[core_id],lineaddr,false,core_id);
    if(p != HIT) 
     {
      delay = delay + memsys_L2_access_multicore(sys,lineaddr,false,core_id);
      cache_install(sys->dcache_coreid[core_id],lineaddr,false,core_id); //install new line in l1
      //install evicted line in l2
      if (sys->dcache_coreid[core_id]->last_evicted_line.Dirty==true && sys->dcache_coreid[core_id]->last_evicted_line.Valid==true)
        {
         (void) memsys_L2_access_multicore(sys,sys->dcache_coreid[core_id]->last_evicted_line.address,true,core_id);
        
        }     
     }  
  }
  
  if(type == ACCESS_TYPE_STORE){
    // YOU NEED TO WRITE THIS PART AND UPDATE DELAY
   
    delay = delay+ DCACHE_HIT_LATENCY;
    p = cache_access(sys->dcache_coreid[core_id],lineaddr,true,core_id); 
     if(p!=HIT) 
     {delay = delay + memsys_L2_access_multicore(sys,lineaddr,false,core_id);
      cache_install(sys->dcache_coreid[core_id],lineaddr,true,core_id);   //install line in l1
      //install evicted line in l2
      if (sys->dcache_coreid[core_id]->last_evicted_line.Dirty==true && sys->dcache_coreid[core_id]->last_evicted_line.Valid==true)
        {
         
 	 (void) memsys_L2_access_multicore(sys,sys->dcache_coreid[core_id]->last_evicted_line.address,true,core_id);
      
        }
     
     }
  
}
 
  return delay;
}


/////////////////////////////////////////////////////////////////////
// This function is called on ICACHE miss, DCACHE miss, DCACHE writeback
// ----- YOU NEED TO WRITE THIS FUNCTION AND UPDATE DELAY ----------
/////////////////////////////////////////////////////////////////////

uns64   memsys_L2_access_multicore(Memsys *sys, Addr lineaddr, Flag is_writeback, uns core_id){
  uns64 delay = L2CACHE_HIT_LATENCY;
	
  //To get the delay of L2 MISS, you must use the dram_access() function
  //To perform writebacks to memory, you must use the dram_access() function
  //This will help us track your memory reads and memory writes
  Flag temp;
  temp = cache_access(sys->l2cache,lineaddr,is_writeback,core_id);
  if(temp==MISS)
   { delay = delay + dram_access(sys->dram,lineaddr,false);
    
     cache_install(sys->l2cache,lineaddr,is_writeback,core_id);
     if (sys->l2cache->last_evicted_line.Dirty==true && sys->l2cache->last_evicted_line.Valid==true)
     { (void)dram_access(sys->dram,sys->l2cache->last_evicted_line.address,true);
     }
   }
	
  return delay;
}


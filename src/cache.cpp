#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cache.h"

extern uns64 SWP_CORE0_WAYS;
////////////////////////////////////////////////////////////////////
// ------------- TRY NOT TO MODIFY THE PRINT STATS FUNCTION -----------
////////////////////////////////////////////////////////////////////

void cache_print_stats(Cache *c, char *header){
  double read_mr =0;
  double write_mr =0;

  if(c->stat_read_access){
    read_mr=(double)(c->stat_read_miss)/(double)(c->stat_read_access);
  }

  if(c->stat_write_access){
    write_mr=(double)(c->stat_write_miss)/(double)(c->stat_write_access);
  }

  printf("\n%s_READ_ACCESS    \t\t : %10llu", header, c->stat_read_access);
  printf("\n%s_WRITE_ACCESS   \t\t : %10llu", header, c->stat_write_access);
  printf("\n%s_READ_MISS      \t\t : %10llu", header, c->stat_read_miss);
  printf("\n%s_WRITE_MISS     \t\t : %10llu", header, c->stat_write_miss);
  printf("\n%s_READ_MISS_PERC  \t\t : %10.3f", header, 100*read_mr);
  printf("\n%s_WRITE_MISS_PERC \t\t : %10.3f", header, 100*write_mr);
  printf("\n%s_DIRTY_EVICTS   \t\t : %10llu", header, c->stat_dirty_evicts);

  printf("\n");
}


Cache* cache_new(uns64 size, uns64 assocs, uns64 linesize, uns64 repl_policy)
{ uns32 num_set = size/(linesize*assocs);
  Cache* p = (Cache*) calloc(1,sizeof(Cache));
  p->cache_set = (Cache_Set*) calloc(num_set,sizeof(Cache_Set));
  uns32 ii=0;
//printf("In cache");
  while(ii<num_set){
   p->cache_set[ii].cache_line = (Cache_Line*) calloc(assocs, sizeof(Cache_Line));ii++;}
  p->num_set = num_set;
  p->num_way = assocs;
  p->repl_policy = repl_policy;
  //printf("New cache");
  return p;
}


Flag cache_access(Cache *c, Addr lineaddr, uns is_write, uns core_id)		//have to do core_id implement
{ //printf("\nstart2");
  int k = log2(c->num_set);
  uns64 t = (lineaddr)>>(k);
  uns64 set_index = lineaddr%((uns)pow(2,k));
 // uns64 set_index = lineaddr&(k-1);

 uns64 ways = c->num_way ;
    uns64 ii = 0;
  while(ii<ways)
       {
         if((c->cache_set[set_index].cache_line[ii].tag == t) && c->cache_set[set_index].cache_line[ii].Valid==true)
         {
            c->cache_set[set_index].cache_line[ii].last_access_time = cycle;
	  if(is_write == true)
           {c->cache_set[set_index].cache_line[ii].Dirty=true;
            c->stat_write_access++;
           }
          else c->stat_read_access++;
          return HIT;
         } ii++;
       }
  if(is_write != false)
   c->stat_write_miss++;
  if(is_write!=true)
   c->stat_read_miss++;
  //printf("cahce access");
  return MISS;
}

void cache_install(Cache *c, Addr lineaddr, uns is_write, uns core_id)		
{ 
 if(is_write!=false)
   c->stat_write_access++;
  else c->stat_read_access++;
  int k = log2(c->num_set);
  uns64 set_index = lineaddr%((uns)pow(2,k));
  uns ptr = cache_find_victim(c,set_index,core_id);
  c->last_evicted_line = c->cache_set[set_index].cache_line[ptr];
  c->last_evicted_line.address =c->cache_set[set_index].cache_line[ptr].tag<<k ^ set_index;		
  if(c->cache_set[set_index].cache_line[ptr].Dirty !=false)
    c->stat_dirty_evicts++;
  
  uns64 t = (lineaddr)>>(k);
  c->cache_set[set_index].cache_line[ptr].tag = t;
   c->cache_set[set_index].cache_line[ptr].Valid = true;
 c->cache_set[set_index].cache_line[ptr].core_id=core_id;

  if(is_write != false)
    c->cache_set[set_index].cache_line[ptr].Dirty=true;   		
  else  c->cache_set[set_index].cache_line[ptr].Dirty = false;
   c->cache_set[set_index].cache_line[ptr].last_access_time = cycle;
 
}

uns cache_find_victim(Cache *c, uns set_index, uns core_id)			
{ 
  if(c->repl_policy == 0) 
   { 
   
     uns temp;
     uns var;
     uns64 ii =0;
     uns64 jj=0;
     bool in_valid;
 /*while(jj<c->num_way)
  {
   if( c->cache_set[set_index].cache_line[jj].Valid==false)
   {
    in_valid=true;
    break;
    }	
   jj++;
  }
 if (in_valid==true)
{
  var=jj;
}
else*/{     
     while(ii< c->num_way)
       { if(ii==0)
           {
	    temp = c->cache_set[set_index].cache_line[ii].last_access_time;
            var = ii;
           }

         if(c->cache_set[set_index].cache_line[ii].last_access_time < temp)
     	   {
        temp = c->cache_set[set_index].cache_line[ii].last_access_time;
            var = ii;
           } ii++;
       }
     return var;
   }
}
 else if(c->repl_policy == 1) 
  { 
    uns var;
    var = rand()%(c->num_way);
    return var;
  }
else if (c->repl_policy == 2)
{
 uns temp;
 uns var;
 uns64 ii=0;
 uns64 jj=0;
 uns64 ways; 
  bool in_valid;
 if(core_id ==0) ways = SWP_CORE0_WAYS;
 else if (core_id == 1) {ways = c->num_way;ii=SWP_CORE0_WAYS;jj=SWP_CORE0_WAYS;}
 /* while(jj<ways)
  {
   if( c->cache_set[set_index].cache_line[ii].Valid==false)
    {
    in_valid=true;
    break;
   jj++;
    }
  }
 if (in_valid==true)
{
  var=jj;
}
else*/{

 while(ii<ways)
 {
   if(ii==0 || ii==SWP_CORE0_WAYS)
     {
	temp = c->cache_set[set_index].cache_line[ii].last_access_time;
        var = ii;
     }
   if(c->cache_set[set_index].cache_line[ii].last_access_time < temp)
     {
	temp = c->cache_set[set_index].cache_line[ii].last_access_time;
        var=ii;
     } ii++;
}
}
 return var;
}
/*if(c->repl_policy ==2)
{
  if (core_id==0) return 0;
  if (core_id==1) return 4;
}
else*/
return 0;
}

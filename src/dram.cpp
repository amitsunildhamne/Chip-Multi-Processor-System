#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dram.h"

#define DRAM_Banks  16
#define ROW_BUFF_SZ 1024
#define ACT 45
#define CAS 45
#define PRE 45
#define BUS 10

extern MODE SIM_MODE;
extern uns64 CACHE_LINESIZE;
extern Flag DRAM_PAGE_POLICY;
////////////////////////////////////////////////////////////////////
// ------------- TRY NOT TO MODIFY THE PRINT STATS FUNCTION -------
///////////////////////////////////////////////////////////////////

DRAM* dram_new()
{
    DRAM* dramMem = (DRAM* )calloc(1, sizeof(DRAM));
    for(int ii =0; ii<256;ii++)
      {
	dramMem->Row_Buffer_Entry[ii].Row_Id=ii;
      }
    return dramMem;
}

uns64  dram_access(DRAM *dram,Addr lineaddr, Flag is_dram_write)
{
  uns64 delay = 100;
  if(SIM_MODE != SIM_MODE_B)
    delay = dram_access_mode_CDE(dram, lineaddr, is_dram_write);

  if(is_dram_write)
  {
    dram->stat_write_access++;
    dram->stat_write_delay = dram->stat_write_delay+delay;
  }
  else
  {
    dram->stat_read_access++;
    dram->stat_read_delay = dram->stat_read_delay+delay;
  }

  return delay;
}

uns64 dram_access_mode_CDE(DRAM *dram, Addr lineaddr, Flag is_dram_write)
{
  uns64 delay_cp = ACT + CAS + BUS;
  uns64 delay_op;
  uns64 offset = ROW_BUFF_SZ / CACHE_LINESIZE;
  uns64 Bank_ID = (lineaddr / offset) % DRAM_Banks;
  uns64 Row_Buf_ID = (lineaddr / offset) / DRAM_Banks;
    //close page policy
 
  //open page policy
  if(DRAM_PAGE_POLICY == 0)
  {
    if(dram->Row_Buffer_Entry[Bank_ID].valid)
    {
      if(dram->Row_Buffer_Entry[Bank_ID].Row_Id != Row_Buf_ID)
      {
        dram->Row_Buffer_Entry[Bank_ID].Row_Id = Row_Buf_ID;
        delay_op = PRE + ACT + CAS + BUS;
      }
      else
        delay_op = CAS + BUS;
    }
    else
    {
      dram->Row_Buffer_Entry[Bank_ID].valid = true;
      dram->Row_Buffer_Entry[Bank_ID].Row_Id = Row_Buf_ID;
      delay_op = ACT + CAS + BUS;
    }
   return delay_op;
  }
 if(DRAM_PAGE_POLICY == 1)
  {
   return  delay_cp;  
  }

 }

void    dram_print_stats(DRAM *dram)
{
  double rddelay_avg=0;
  double wrdelay_avg=0;
  char header[256];
  sprintf(header, "DRAM");

  if(dram->stat_read_access){
    rddelay_avg=(double)(dram->stat_read_delay)/(double)(dram->stat_read_access);
  }

  if(dram->stat_write_access){
    wrdelay_avg=(double)(dram->stat_write_delay)/(double)(dram->stat_write_access);
  }
  printf("\n%s_READ_ACCESS\t\t : %10llu", header, dram->stat_read_access);

  printf("\n%s_WRITE_ACCESS\t\t : %10llu", header, dram->stat_write_access);
  printf("\n%s_READ_DELAY_AVG\t\t : %10.3f", header, rddelay_avg);
  printf("\n%s_WRITE_DELAY_AVG\t\t : %10.3f", header, wrdelay_avg);


}

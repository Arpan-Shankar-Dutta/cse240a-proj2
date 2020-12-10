//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <math.h>

//
// TODO:Student Information
//
const char *studentName = "Arpan Shankar Dutta";
const char *studentID   = "A53276227";
const char *email       = "adutta@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

//struct cache definition in cache.h

cache *icache;
cache *dcache;
cache *l2cache;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //

  uint32_t i,j;

  icache = malloc(icacheSets*sizeof(cache));                  //initialise Icache

  for(i=0;i<icacheSets;i++)
  {
    (icache+i)->valid = malloc(icacheAssoc*sizeof(uint8_t));
    (icache+i)->LRU = malloc(icacheAssoc*sizeof(uint8_t));
    (icache+i)->tag = malloc(icacheAssoc*sizeof(uint32_t));
  }
  
  for(i=0;i<icacheSets;i++)
  {
    for(j=0;j<icacheAssoc;j++)
    {
      (icache+i)->valid[j] = 0;
      (icache+i)->LRU[j] = j;
      (icache+i)->tag[j] = 0;
    }
  }

  dcache = malloc(dcacheSets*sizeof(cache));                  //initialise Dcache

  for(i=0;i<dcacheSets;i++)
  {
    (dcache+i)->valid = malloc(dcacheAssoc*sizeof(uint8_t));
    (dcache+i)->LRU = malloc(dcacheAssoc*sizeof(uint8_t));
    (dcache+i)->tag = malloc(dcacheAssoc*sizeof(uint32_t));
  }
  
  for(i=0;i<dcacheSets;i++)
  {
    for(j=0;j<dcacheAssoc;j++)
    {
      (dcache+i)->valid[j] = 0;
      (dcache+i)->LRU[j] = j;
      (dcache+i)->tag[j] = 0;
    }
  }

  l2cache = malloc(l2cacheSets*sizeof(cache));                //Initialise L2 cache

  for(i=0;i<l2cacheSets;i++)
  {
    (l2cache+i)->valid = malloc(l2cacheAssoc*sizeof(uint8_t));
    (l2cache+i)->LRU = malloc(l2cacheAssoc*sizeof(uint8_t));
    (l2cache+i)->tag = malloc(l2cacheAssoc*sizeof(uint32_t));
  }
  
  for(i=0;i<l2cacheSets;i++)
  {
    for(j=0;j<l2cacheAssoc;j++)
    {
      (l2cache+i)->valid[j] = 0;
      (l2cache+i)->LRU[j] = j;
      (l2cache+i)->tag[j] = 0;
    }
  }
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //

  if(icacheSets!=0)
  {
    icacheRefs++;

    uint32_t j, penalty, hit = 0, used_entry, i_penalty, invalid = 0, invalid_entry;

    uint8_t addr_shift_index = log2(blocksize);
    uint8_t addr_shift_tag = log2(blocksize*icacheSets);
    uint16_t index = (addr>>addr_shift_index)&(icacheSets - 1);
    uint32_t tag = addr>>addr_shift_tag;

    for(j=0;j<icacheAssoc;j++)
    {
      if( (icache + index)->tag[j]==tag && (icache + index)->valid[j] == 1)
      {
        hit = 1;
        used_entry = j;
        break;
      }
    }

    if(hit)
    {
      for(j=0;j<icacheAssoc;j++)
      {
        if((icache+index)->LRU[j]<(icache+index)->LRU[used_entry])
        {
          (icache+index)->LRU[j]++;
        }
      }
    
      (icache+index)->LRU[used_entry] = 0;

      penalty = icacheHitTime;
    }
    else
    {
      for(j=0;j<icacheAssoc;j++)
      {
        if( (icache+index)->valid[j]==0 )
        {
          (icache+index)->valid[j] = 1;
          (icache+index)->tag[j] = tag;
          invalid = 1;
          invalid_entry = j;
          break;
        }
      }

      if(invalid)
      {
        for(j=0;j<icacheAssoc;j++)
        {
          if((icache+index)->LRU[j]<(icache+index)->LRU[invalid_entry])
          {
            (icache+index)->LRU[j]++;
          }
        }
    
        (icache+index)->LRU[invalid_entry] = 0;
      }
      else
      {
        for(j=0;j<icacheAssoc;j++)
        {
          if( (icache+index)->LRU[j]==icacheAssoc-1 )
          {
            (icache+index)->valid[j] = 1;
            (icache+index)->tag[j] = tag;
            (icache+index)->LRU[j] = 0;
          }
          else
          {
            (icache+index)->LRU[j]++;
          }
        }
      }

      icacheMisses++;

      i_penalty = l2cache_access(addr);
      icachePenalties += i_penalty;
      penalty = icacheHitTime + i_penalty;
    }

    return penalty;
  }
  else
  {
    uint32_t penalty = l2cache_access(addr);
    
    return penalty;
  }
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //

  if(dcacheSets!=0)
  {
    dcacheRefs++;

    uint32_t j, penalty, hit = 0, used_entry, i_penalty, invalid = 0, invalid_entry;

    uint8_t addr_shift_index = log2(blocksize);
    uint8_t addr_shift_tag = log2(blocksize*dcacheSets);
    uint16_t index = (addr>>addr_shift_index)&(dcacheSets - 1);
    uint32_t tag = addr>>addr_shift_tag;

    for(j=0;j<dcacheAssoc;j++)
    {
      if( (dcache + index)->tag[j]==tag && (dcache + index)->valid[j] == 1)
      {
        hit = 1;
        used_entry = j;
        break;
      }
    }

    if(hit)
    {
      for(j=0;j<dcacheAssoc;j++)
      {
        if((dcache+index)->LRU[j]<(dcache+index)->LRU[used_entry])
        {
          (dcache+index)->LRU[j]++;
        }
      }
    
      (dcache+index)->LRU[used_entry] = 0;

      penalty = dcacheHitTime;
    }
    else
    {
      for(j=0;j<dcacheAssoc;j++)
      {
        if( (dcache+index)->valid[j]==0 )
        {
          (dcache+index)->valid[j] = 1;
          (dcache+index)->tag[j] = tag;
          invalid = 1;
          invalid_entry = j;
          break;
        }
      }

      if(invalid)
      {
        for(j=0;j<dcacheAssoc;j++)
        {
          if((dcache+index)->LRU[j]<(dcache+index)->LRU[invalid_entry])
          {
            (dcache+index)->LRU[j]++;
          }
        }
    
        (dcache+index)->LRU[invalid_entry] = 0;
      }
      else
      {
        for(j=0;j<dcacheAssoc;j++)
        {
          if( (dcache+index)->LRU[j]==dcacheAssoc-1 )
          {
            (dcache+index)->valid[j] = 1;
            (dcache+index)->tag[j] = tag;
            (dcache+index)->LRU[j] = 0;
          }
          else
          {
            (dcache+index)->LRU[j]++;
          }
        }
      }

      dcacheMisses++;

      i_penalty = l2cache_access(addr);
      dcachePenalties += i_penalty;
      penalty = dcacheHitTime + i_penalty;
    }

    return penalty;
  }
  else
  {
    uint32_t penalty = l2cache_access(addr);
      
    return penalty;
  }
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //

  if(l2cacheSets!=0)
  {
    l2cacheRefs++;

    uint32_t j, penalty, hit = 0, used_entry, i_penalty, invalid = 0, invalid_entry, tag_evict, addr_evict;

    uint8_t addr_shift_index = log2(blocksize);
    uint8_t addr_shift_tag = log2(blocksize*l2cacheSets);
    uint16_t index = (addr>>addr_shift_index)&(l2cacheSets - 1);
    uint32_t tag = addr>>addr_shift_tag;

    for(j=0;j<l2cacheAssoc;j++)
    {
      if( (l2cache + index)->tag[j]==tag && (l2cache + index)->valid[j] == 1)
      {
        hit = 1;
        used_entry = j;
        break;
      }
    }

    if(hit)
    {
      for(j=0;j<l2cacheAssoc;j++)
      {
        if((l2cache+index)->LRU[j]<(l2cache+index)->LRU[used_entry])
        {
          (l2cache+index)->LRU[j]++;
        }
      }
    
      (l2cache+index)->LRU[used_entry] = 0;

      penalty = l2cacheHitTime;
    }
    else
    {
      for(j=0;j<l2cacheAssoc;j++)
      {
        if( (l2cache+index)->valid[j]==0 )
        {
          (l2cache+index)->valid[j] = 1;
          (l2cache+index)->tag[j] = tag;
          invalid = 1;
          invalid_entry = j;
          break;
        }
      }

      if(invalid)
      {
        for(j=0;j<l2cacheAssoc;j++)
        {
          if((l2cache+index)->LRU[j]<(l2cache+index)->LRU[invalid_entry])
          {
            (l2cache+index)->LRU[j]++;
          }
        }
    
        (l2cache+index)->LRU[invalid_entry] = 0;
      }
      else
      {
        for(j=0;j<l2cacheAssoc;j++)
        {
          if( (l2cache+index)->LRU[j]==l2cacheAssoc-1 )
          {
            (l2cache+index)->valid[j] = 1;
            tag_evict = (l2cache+index)->tag[j];
            (l2cache+index)->tag[j] = tag;
            (l2cache+index)->LRU[j] = 0;
          }
          else
          {
            (l2cache+index)->LRU[j]++;
          }
        }

        if(inclusive)
        {
          addr_evict = (tag_evict<<addr_shift_tag) + (index<<addr_shift_index);

          uint8_t addr_shift_index_i = log2(blocksize);
          uint8_t addr_shift_tag_i = log2(blocksize*icacheSets);
          uint16_t index_i = (addr_evict>>addr_shift_index_i)&(icacheSets - 1);
          uint32_t tag_i = addr_evict>>addr_shift_tag_i;

          for(j=0;j<icacheAssoc;j++)
          {
            if((icache + index_i)->tag[j]==tag_i)
            {
              (icache + index_i)->valid[j] = 0;
              break;
            }
          }

          uint8_t addr_shift_index_d = log2(blocksize);
          uint8_t addr_shift_tag_d = log2(blocksize*dcacheSets);
          uint16_t index_d = (addr_evict>>addr_shift_index_d)&(dcacheSets - 1);
          uint32_t tag_d = addr_evict>>addr_shift_tag_d;

          for(j=0;j<dcacheAssoc;j++)
          {
            if((dcache + index_d)->tag[j]==tag_d)
            {
              (dcache + index_d)->valid[j] = 0;
              break;
            }
          }
        }
      }

      l2cacheMisses++;

      i_penalty = memspeed;
      l2cachePenalties += i_penalty;
      penalty = l2cacheHitTime + i_penalty;
    }

    return penalty;
  }
  else
  {
    uint32_t penalty = memspeed;
        
    return penalty;
  }
}
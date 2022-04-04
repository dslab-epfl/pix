/* Contracts for map-impl functions for the Rehashing map implementation */

#include "map-impl-contracts.h"

/* Perf contracts */

long map_impl_init_contract(std::string metric, long success, long capacity)
{
  long constant;
  if (metric == "instruction count")
    constant = 11 + 5 * capacity;
  else if (metric == "memory instructions")
    constant = 5 + 2 * capacity;
  else if (metric == "execution cycles")
    constant = 3 * DRAM_LATENCY + 2 * L1_LATENCY + 6 +
               ((2 * DRAM_LATENCY) / 16 + 3) * capacity;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}
long map_impl_put_contract(std::string metric, long num_traversals)
{
  long constant, dynamic;
  if (metric == "instruction count")
  {
    dynamic = 0;
    if (num_traversals == 1)
    {
      constant = 43;
    }
    else
    {
      constant = 42;
      dynamic = 14 * (num_traversals - 1);
    }
  }
  else if (metric == "memory instructions")
  {
    dynamic = 0;
    if (num_traversals == 1)
    {
      constant = 23;
    }
    else
    {
      constant = 23;
      dynamic = 3 * (num_traversals - 1);
    }
  }
  else if (metric == "execution cycles")
  {
    constant = 6 * DRAM_LATENCY + 14 * L1_LATENCY +
               35; // Have not gone through patterns here. In progress
    dynamic = 0;
    if (num_traversals > 1)
    {
      num_traversals--;
      dynamic = ((2 * num_traversals) / 16 + 2) * DRAM_LATENCY +
                (((30 * num_traversals) / 16) - 2) * L1_LATENCY +
                18 * num_traversals; // The additional 2 is for misalignments
    }
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant + dynamic;
}

long map_impl_get_contract(std::string metric, long success,
                           long num_traversals, long num_collisions)
{
  long constant, dynamic;
  constant = 0;
  dynamic = 0;
  long rem_traversals;
  if (metric == "instruction count")
  {
    if (success)
    {
      if (num_traversals == 1)
      {
        constant = 65 + int_key_eq_contract(metric, success);
      }
      else
      {
        constant = 65 + int_key_eq_contract(metric, success);
        dynamic += num_collisions * (28 + int_key_eq_contract(metric, 0));
        rem_traversals = num_traversals - num_collisions - 1;
        dynamic += rem_traversals * 18;
      }
    }
    else
    {
      if (num_collisions == 0 &&
          num_traversals != 65536) // This should be capacity
      {
        constant = 48;
      }
      else if (num_traversals != 65536)
      {
        constant = 49;
      }
      else
      {
        constant = 52;
      }
      dynamic += num_collisions * (28 + int_key_eq_contract(metric, 0));
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 18;
    }
  }
  else if (metric == "memory instructions")
  {
    if (success)
    {
      if (num_traversals == 1)
      {
        constant = 37 + int_key_eq_contract(metric, success);
      }
      else
      {
        constant = 37 + int_key_eq_contract(metric, success);
        dynamic += num_collisions * (14 + int_key_eq_contract(metric, 0));
        rem_traversals = num_traversals - num_collisions - 1;
        dynamic += rem_traversals * 4;
      }
    }
    else
    {
      if (num_collisions == 0 &&
          num_traversals != 65536) // This should be capacity
      {
        constant = 22;
      }
      else if (num_traversals != 65536)
      {
        constant = 23;
      }
      else
      {
        constant = 23;
      }
      dynamic += num_collisions * (14 + int_key_eq_contract(metric, 0));
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 4;
    }
  }
  else if (metric == "execution cycles")
  {
    if (success)
    {
      constant = 12 * DRAM_LATENCY + 23 * L1_LATENCY +
                 31; // Have not gone through patterns here. In progress
    }
    else
    {
      constant = 4 * DRAM_LATENCY + 18 * L1_LATENCY +
                 29; // Can still optimise across static instances
    }
    dynamic = 0;
    if (num_traversals > 1)
    {
      num_traversals--;
      dynamic = ((3 * num_traversals) / 16 + 2) * DRAM_LATENCY +
                (((45 * num_traversals) / 16) - 2) * L1_LATENCY +
                18 * num_traversals; // The additional 2 is for misalignments
    }
    if (num_collisions > 0)
    {
      dynamic += 1 * DRAM_LATENCY + ((1 * num_collisions) / 16) * DRAM_LATENCY +
                 (24 * num_collisions) * L1_LATENCY +
                 15 * num_collisions; // Keys are 16B, so always cache aligned
    }
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant + dynamic;
}

long map_impl_erase_contract(std::string metric, long num_traversals,
                             long num_collisions)
{
  long constant, dynamic;
  constant = 0;
  dynamic = 0;
  long rem_traversals;
  if (metric == "instruction count")
  {
    if (num_traversals == 1)
    {
      constant = 69 + int_key_eq_contract(metric, 1);
    }
    else
    {
      constant = 69 + int_key_eq_contract(metric, 1);
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (39 + int_key_eq_contract(metric, 0));
      dynamic += 19 * rem_traversals;
    }
  }
  else if (metric == "memory instructions")
  {
    if (num_traversals == 1)
    {
      constant = 43 + int_key_eq_contract(metric, 1);
    }
    else
    {
      constant = 43 + int_key_eq_contract(metric, 1);
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (21 + int_key_eq_contract(metric, 0));
      dynamic += 4 * rem_traversals;
    }
  }
  else if (metric == "execution cycles")
  {
    constant = 11 * DRAM_LATENCY + 51 * L1_LATENCY + 31;
    if (num_traversals > 1)
    {
      num_traversals--;
      dynamic = ((3 * num_traversals) / 16 + 2) * DRAM_LATENCY +
                (((61 * num_traversals) / 16) - 2) * L1_LATENCY +
                17 * num_traversals;
    }
    if (num_collisions > 0)
    {
      dynamic += 1 * DRAM_LATENCY + ((1 * num_collisions) / 16) * DRAM_LATENCY +
                 (24 * num_collisions) * L1_LATENCY +
                 15 * num_collisions; // Keys are 16B, so always cache aligned
    }
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant + dynamic;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_impl_init_cstate_contract(long success, long capacity)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
map_impl_put_cstate_contract(long num_traversals)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_impl_get_cstate_contract(long success, long num_traversals,
                             long num_collisions)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_impl_erase_cstate_contract(long num_traversals, long num_collisions)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

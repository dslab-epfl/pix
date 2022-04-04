#include "map-contracts.h"

#define REHASHING_THRESHOLD 100

/* Perf contracts */

long map_allocate_contract_0(std::string metric, std::vector<long> values)
{
  return 0;
}

long map_allocate_contract_1(std::string metric, std::vector<long> values)
{
  return 0;
}

long map_get_contract_0(std::string metric, std::vector<long> values)
{
  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long constant, dependency;
  assert(success);
  if (metric == "instruction count")
  {
    constant = 30;
  }
  else if (metric == "memory instructions")
  {
    constant = 24;
  }
  else if (metric == "execution cycles")
  {
    constant = 12 * DRAM_LATENCY + 12 * L1_LATENCY +
               8; // Have not gone through patterns here. In progress
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency =
      map_impl_get_contract(metric, success, num_traversals, num_collisions);
  dependency += int_key_hash_contract(metric);
  return constant + dependency;
}
long map_get_contract_1(std::string metric, std::vector<long> values)
{
  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long constant, dependency;
  assert(success == 0);
  if (metric == "instruction count")
  {
    constant = 30;
  }
  else if (metric == "memory instructions")
  {
    constant = 24;
  }
  else if (metric == "execution cycles")
  {
    constant = 12 * DRAM_LATENCY + 12 * L1_LATENCY +
               8; // Have not gone through patterns here. In progress
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency =
      map_impl_get_contract(metric, success, num_traversals, num_collisions);
  dependency += int_key_hash_contract(metric);
  return constant + dependency;
}

long map_put_contract_0(std::string metric, std::vector<long> values)
{
  long num_traversals = values[0];
  long map_occupancy = values[1];
  long constant, dynamic, constant_dependency, dynamic_dependency;
  if (num_traversals > REHASHING_THRESHOLD)
  {

    if (metric == "instruction count")
    {
      constant = 90;
      dynamic = 10 * 65536;
      dynamic += 34 * map_occupancy;
    }
    else if (metric == "memory instructions")
    {
      constant = 58;
      dynamic = 4 * 65536;
      dynamic += 24 * map_occupancy;
    }
    else if (metric == "execution cycles")
    {
      constant = 7 * DRAM_LATENCY + 12 * L1_LATENCY +
                 9; // Have not gone through patterns here. In progress
      dynamic = (2 * DRAM_LATENCY + 2 * L1_LATENCY + 6) * 65536;
      dynamic += (12 * DRAM_LATENCY + 12 * L1_LATENCY + 10) * map_occupancy;
    }
    else {
    assert( 0 && "Contract does not support this metric");
  }
    constant_dependency = map_impl_put_contract(metric, num_traversals);
    constant_dependency += int_key_hash_contract(metric);
    constant_dependency += map_impl_init_contract(metric, 1, 65536);
    dynamic_dependency =
        map_impl_put_contract(metric, num_traversals) * map_occupancy;
    dynamic_dependency += int_key_hash_contract(metric) * map_occupancy;
  }
  else
  {
    if (metric == "instruction count")
    {
      constant = 53;
    }
    else if (metric == "memory instructions")
    {
      constant = 38;
    }
    else if (metric == "execution cycles")
    {
      constant = 19 * DRAM_LATENCY + 19 * L1_LATENCY +
                 17; // Have not gone through patterns here. In progress
    }
    else {
    assert( 0 && "Contract does not support this metric");
  }
    dynamic = 0;
    constant_dependency = map_impl_put_contract(metric, num_traversals);
    constant_dependency += int_key_hash_contract(metric);
    dynamic_dependency = 0;
  }

  return constant + dynamic + constant_dependency + dynamic_dependency;
}

long map_erase_contract_0(std::string metric, std::vector<long> values)
{
  long num_traversals = values[0];
  long num_collisions = values[1];
  long constant, dependency;
  if (metric == "instruction count")
  {
    constant = 33;
  }
  else if (metric == "memory instructions")
  {
    constant = 26;
  }
  else if (metric == "execution cycles")
  {
    constant = 14 * DRAM_LATENCY + 12 * L1_LATENCY + 9; // Needs to be updated
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency = map_impl_erase_contract(metric, num_traversals, num_collisions);
  dependency += int_key_hash_contract(metric);
  return constant + dependency;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_allocate_cstate_contract_0(std::vector<long> values)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_allocate_cstate_contract_1(std::vector<long> values)
{

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_get_cstate_contract_0(std::vector<long> values)
{
  long success = values[2];
  assert(success);

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
map_get_cstate_contract_1(std::vector<long> values)
{
  long success = values[2];
  assert(success == 0);

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_put_cstate_contract_0(std::vector<long> values)
{
  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_erase_cstate_contract_0(std::vector<long> values)
{
  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
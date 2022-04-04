#pragma once

#include <rte_ip.h>


#include "lpm.h"

void lpm_init(const char fname[], void **lpm_out);
uint32_t lpm_lookup(void *lpm, uint32_t addr);

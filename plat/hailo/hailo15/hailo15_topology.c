/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <common/debug.h>

#include <plat/common/platform.h>

#include <arch.h>

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int core_pos;

    core_pos = MPIDR_AFFLVL0_VAL(mpidr);

	if ((MPIDR_AFFLVL2_VAL(mpidr) > 0) ||
	    (MPIDR_AFFLVL1_VAL(mpidr) > 0) ||
	    (core_pos >= PLATFORM_CORE_COUNT)) {
		ERROR ("Invalid mpidr: 0x%08x\n", (uint32_t)mpidr);
		panic();
	}

	return core_pos;
}

static const unsigned char plat_power_domain_tree_desc[] = {1, 4};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}

/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_DYN_CFG_GETTER_H
#define FCONF_DYN_CFG_GETTER_H

#include <lib/fconf/fconf.h>

#define FCONF_INVALID_IDX	0xFFFFFFFFU

/* Dynamic configuration related getter */
#define dyn_cfg__dtb_getter(id)	dyn_cfg_dtb_info_getter(id)

struct dyn_cfg_dtb_info_t {
	uintptr_t config_addr;
	uint32_t config_max_size;
	unsigned int config_id;
	/*
	 * A platform uses this address to copy the configuration
	 * to another location during the boot-flow.
	 * - e.g. HW_CONFIG
	 */
	uintptr_t secondary_config_addr;
};

unsigned int dyn_cfg_dtb_info_get_index(unsigned int config_id);
struct dyn_cfg_dtb_info_t *dyn_cfg_dtb_info_getter(unsigned int config_id);
int fconf_populate_dtb_registry(uintptr_t config);

/* Set config information in global DTB array */
void set_config_info(uintptr_t config_addr, uintptr_t secondary_config_addr,
		     uint32_t config_max_size,
		     unsigned int config_id);

#endif /* FCONF_DYN_CFG_GETTER_H */

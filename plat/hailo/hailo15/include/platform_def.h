/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>


#define PLATFORM_STACK_SIZE 0x1000
#define CACHE_WRITEBACK_GRANULE 64
#define PLATFORM_CORE_COUNT	4


/* TODO: fill actual numbers for power states */
#define PLAT_NUM_PWR_DOMAINS	5
#define PLAT_MAX_PWR_LVL	1
#define PLAT_MAX_OFF_STATE	2
#define PLAT_MAX_RET_STATE	1

#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 35)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 35)
#define PLAT_NS_IMAGE_OFFSET	0x83000000
#define PLAT_CPU_RELEASE_ADDR	0x800FF000
#define SECONDARY_CPU_SPIN_BASE_ADDR	(PLAT_CPU_RELEASE_ADDR + 8)
#define CPU_RELEASE_SUCCESS_TABLE	(SECONDARY_CPU_SPIN_BASE_ADDR + 8 * PLATFORM_CORE_COUNT)

#define BL31_BASE	ULL(0x80000000)
#define BL31_SIZE	ULL(0x000FF000)
#define BL31_LIMIT	(BL31_BASE + BL31_SIZE)

/* TODO: find optimal values for those 2 */
#define MAX_XLAT_TABLES	8
#define MAX_MMAP_REGIONS	32

#define GICD_BASE	0x60600000
#define GICD_SIZE	0x10000

#define GICR_BASE	0x60680000
#define GICR_SIZE	0x80000

#define UART1_BASE	0x109000
#define UART_BAUDRATE	115200

/* TODO: should be received from SCMI */
#define UART_CLOCK	200000000
#define PLAT_SYSCNT_FREQ 25000000

#endif /* PLATFORM_DEF_H */

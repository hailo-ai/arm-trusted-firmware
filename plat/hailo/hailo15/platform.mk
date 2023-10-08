#
# Copyright (c) 2023, Hailo Technologies Ltd. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

RESET_TO_BL31		:=	1
ERRATA_A53_855873	:=	1
ERRATA_A53_1530924	:=	1
PLAT_INCLUDES		:=	-Iplat/hailo/hailo15/include

BL31_SOURCES += plat/hailo/hailo15/hailo15_helpers.S \
		plat/hailo/hailo15/hailo15_topology.c \
		plat/hailo/hailo15/hailo15_psci.c \
		plat/hailo/hailo15/hailo15_bl31_plat_setup.c \
		plat/common/plat_psci_common.c \
		plat/common/plat_gicv3.c \
		${GICV3_SOURCES} \
		plat/common/aarch64/crash_console_helpers.S \
		${XLAT_TABLES_LIB_SRCS} \
		lib/cpus/aarch64/cortex_a53.S \
		drivers/ti/uart/aarch64/16550_console.S \
		drivers/arm/css/scmi/scmi_common.c \
		drivers/arm/css/scmi/scmi_pwr_dmn_proto.c \
		drivers/arm/css/scmi/scmi_sys_pwr_proto.c

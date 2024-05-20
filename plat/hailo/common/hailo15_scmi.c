#include <common/debug.h>
#include <stdint.h>
#include <plat/common/platform.h>
#include <drivers/arm/css/scmi.h>
#include <lib/mmio.h>

#include "plat_private.h"

/*
 * The global handle for invoking the SCMI driver APIs after the driver
 * has been initialized.
 */
static void *scmi_handle;

/* The SCMI channel global object */
static scmi_channel_t scmi_channel;

/*
 * This file implements the SCP helper functions using SCMI protocol.
 */

DEFINE_BAKERY_LOCK(hailo_scmi_lock);

static void hailo_scmi_ring_doorbell(struct scmi_channel_plat_info *plat_info);

static scmi_channel_plat_info_t scmi_plat_info =
{
	.scmi_mbx_mem = PLAT_SCMI_CHANNEL_SHMEM_ADDR,
	.db_reg_addr = HAILO_MAILBOX_CPU0_W1S_ADDR,
	.db_preserve_mask = HAILO_MAILBOX_SCMI_CHANNEL_PRESERVE_MASK,
	.db_modify_mask = HAILO_MAILBOX_SCMI_CHANNEL_MODIFY_MASK,
	.ring_doorbell = &hailo_scmi_ring_doorbell,
};

static void hailo_scmi_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	mmio_write_32(plat_info->db_reg_addr, plat_info->db_modify_mask);
}

void hailo_scmi_init(void)
{
	scmi_channel.info = &scmi_plat_info;
	scmi_channel.lock = &hailo_scmi_lock;
	scmi_handle = scmi_init(&scmi_channel);
	if (scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
}

static void __dead2 hailo_scmi_system_off(int state)
{
	int ret;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt from waking
	 * up the AP from WFI.
	 */
	hailo_gic_cpuif_disable();

	/*
	 * Issue SCMI command. First issue a graceful
	 * request and if that fails force the request.
	 */
	ret = scmi_sys_pwr_state_set(scmi_handle,
			SCMI_SYS_PWR_FORCEFUL_REQ,
			state);

	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI system power state set 0x%x returns unexpected 0x%x\n",
			state, ret);
		panic();
	}
	wfi();
	ERROR("SCMI set power state: operation not handled.\n");
	panic();
}

/*
 * Helper function to reset the system via SCMI.
 */
void __dead2 hailo_scmi_sys_shutdown(void)
{
	hailo_scmi_system_off(SCMI_SYS_PWR_SHUTDOWN);
}

void __dead2 hailo_scmi_sys_reboot(void)
{
	hailo_scmi_system_off(SCMI_SYS_PWR_COLD_RESET);
}

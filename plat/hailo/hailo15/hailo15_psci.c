#include <common/debug.h>
#include <stdint.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include "plat_private.h"

static uintptr_t plat_sec_entrypoint;

static int plat_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu = plat_core_pos_by_mpidr(mpidr);

	if (!((0 < cpu) && (cpu <= 3))) {
		return PSCI_E_INVALID_PARAMS;
	}

	secondary_release_addresses[cpu] = plat_sec_entrypoint;

	dsbsy();

	/* send event to all cores */
	sev();

	return PSCI_E_SUCCESS;
}

static void plat_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	hailo15_gic_my_core_init();
}

const plat_psci_ops_t hailo15_psci_ops = {
	.pwr_domain_on = plat_pwr_domain_on,
	.pwr_domain_on_finish = plat_pwr_domain_on_finish,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &hailo15_psci_ops;
	plat_sec_entrypoint = sec_entrypoint;
	return 0;
}

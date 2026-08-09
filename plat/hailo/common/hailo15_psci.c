#include <common/debug.h>
#include <stdint.h>
#include <arch.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include "plat_private.h"

static uintptr_t plat_sec_entrypoint;

/* Marker stamped into the BL31 scratch page so a resumed core warm-boots. */
struct warm_boot_info {
	uint64_t magic;
	uint64_t entrypoint;
};
static volatile struct warm_boot_info * const warm_boot_info =
	(volatile struct warm_boot_info *)PLAT_WARM_BOOT_INFO_ADDR;

/*
 * Disable this core's timer event stream before it is suspended/offlined.
 * The event stream is driven by the system counter and is not a GIC interrupt,
 * so it keeps tickling the cluster (waking the pen WFE, generating activity),
 * and wedges the resume. Runs on the core being powered down, so it hits
 * that core's own CNTKCTL. Linux re-establishes CNTKCTL when the core comes back
 * (hotplug start / resume).
 */
static void hailo_disable_timer_event_stream(void)
{
	uint64_t cntkctl;

	__asm__ volatile("mrs %0, cntkctl_el1" : "=r"(cntkctl));
	cntkctl &= ~((uint64_t)EVNTEN_BIT);
	__asm__ volatile("msr cntkctl_el1, %0" : : "r"(cntkctl));
	isb();
}

static int plat_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu = plat_core_pos_by_mpidr(mpidr);

	INFO("PSCI: core ON request mpidr=0x%lx cpu=%u\n",
	     (unsigned long)mpidr, cpu);

	if (cpu == 0) {
		ERROR("PSCI: core ON rejected: cannot power on primary core\n");
		return PSCI_E_INVALID_PARAMS;
	}
	if (cpu >= PLATFORM_CORE_COUNT) {
		ERROR("PSCI: core ON rejected: cpu %u out of range (max %u)\n",
		      cpu, PLATFORM_CORE_COUNT - 1U);
		return PSCI_E_INVALID_PARAMS;
	}
	secondary_release_addresses[cpu] = plat_sec_entrypoint;

	dsbsy();

	/* send event to all cores */
	sev();

	INFO("PSCI: core %u released to warm-boot entrypoint\n", cpu);

	return PSCI_E_SUCCESS;
}

static void plat_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	hailo15_gic_my_core_init();
	INFO("PSCI: core %u ON finished\n", plat_my_core_pos());
}

static void plat_pwr_domain_off(const psci_power_state_t *target_state)
{
	INFO("PSCI: core %u OFF request\n", plat_my_core_pos());

	hailo_disable_timer_event_stream();

	/* Stop the core being turned off from taking interrupts. */
	hailo_gic_cpuif_disable();
}

static void __dead2 plat_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	unsigned int cpu = plat_my_core_pos();

	if (cpu != 0U) {
		/*
		 * Secondary core: return it to the boot holding pen so a later
		 * PSCI CPU_ON re-releases it through the normal warm-boot path.
		 */
		INFO("PSCI: core %u power-down WFI (returning to holding pen)\n",
		     cpu);
		hailo_secondary_pwr_down();
	}

	/*
	 * Primary core (e.g. system suspend): there is no holding pen to
	 * return to, so just enter the power-down wfi.
	 */
	INFO("PSCI: primary core power-down WFI (system suspend)\n");
	dsbsy();
	for (;;) {
		wfi();
	}
}

static void __dead2 hailo_system_off(void)
{
	NOTICE("PSCI: system off entered on core %u\n", plat_my_core_pos());
	hailo_scmi_sys_shutdown();
}

static void __dead2 hailo_system_reset(void)
{
	NOTICE("PSCI: system reset entered on core %u\n", plat_my_core_pos());
	hailo_scmi_sys_reboot();
}

/*
 * Request the deepest (system-off) local state at every power level. PSCI
 * forces system suspend to be a power-down state, so the top level must report
 * an off state (see psci_system_suspend()).
 */
static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int lvl;

	for (lvl = 0U; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		req_state->pwr_domain_state[lvl] = PLAT_MAX_OFF_STATE;
	}
}

static void plat_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int cpu = plat_my_core_pos();

	INFO("PSCI: system suspend on core %u - notifying SCU via SCMI\n", cpu);

	hailo_disable_timer_event_stream();

	/*
	 * Arm the warm-boot marker before powering down: when the SCU resets this
	 * core to resume, it re-enters BL31 cold and plat_get_my_entrypoint reads
	 * this to redirect to the PSCI warm-boot entrypoint.
	 */
	warm_boot_info->entrypoint = (uint64_t)plat_sec_entrypoint;
	warm_boot_info->magic = PLAT_WARM_BOOT_MAGIC;
	dsbsy();

	hailo_scmi_sys_suspend();

	hailo_gic_cpuif_disable();
}

static void plat_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	hailo_gic_cpuif_enable();

	INFO("PSCI: core %u resumed from system suspend\n", plat_my_core_pos());
}

const plat_psci_ops_t hailo15_psci_ops = {
	.pwr_domain_on = plat_pwr_domain_on,
	.pwr_domain_on_finish = plat_pwr_domain_on_finish,
	.pwr_domain_off		= plat_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = plat_pwr_domain_pwr_down_wfi,
	.pwr_domain_suspend	= plat_pwr_domain_suspend,
	.pwr_domain_suspend_finish = plat_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = plat_get_sys_suspend_power_state,
	.system_off		= hailo_system_off,
	.system_reset		= hailo_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	hailo_scmi_init();   
	*psci_ops = &hailo15_psci_ops;
	plat_sec_entrypoint = sec_entrypoint;
	return 0;
}

#include <common/debug.h>
#include <common/bl_common.h>
#include <platform_def.h>
#include <drivers/ti/uart/uart_16550.h>
#include <drivers/arm/gicv3.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include "plat_private.h"

static console_t console;
static entry_point_info_t bl33_image_ep_info;

static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const gicv3_driver_data_t plat_gic_data = {
	.gicd_base = GICD_BASE,
	.gicr_base = GICR_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = mpidr_to_core_pos
};

volatile uint64_t * const cpu_release_address = (uint64_t *)PLAT_CPU_RELEASE_ADDR;
volatile uint64_t * const secondary_release_addresses = (uint64_t *)SECONDARY_CPU_SPIN_BASE_ADDR;
volatile uint64_t * const cpu_release_succeeded = (uint64_t *)CPU_RELEASE_SUCCESS_TABLE;

void plat_secondary_cpus_bl31_entry(void);

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	console_16550_register(UART1_BASE, UART_CLOCK, UART_BAUDRATE, &console);

	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);

	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

const mmap_region_t plat_regions[] = {
	MAP_REGION_FLAT(UART1_BASE, PAGE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GICD_BASE, GICD_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GICR_BASE, GICR_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PLAT_CPU_RELEASE_ADDR, PAGE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
				MT_DEVICE | MT_RW | MT_SECURE),
		{0}
	};

	setup_page_tables(bl_regions, plat_regions);

	enable_mmu_el3(0);
}

static void bl31_setup_secondary_cpus(void)
{
	for (int i = 1; i < PLATFORM_CORE_COUNT; i++) {
		secondary_release_addresses[i] = 0;
		cpu_release_succeeded[i] = false;
	}

	*cpu_release_address = (uint64_t) plat_secondary_cpus_bl31_entry;

	dsbsy();

	/* send event to all cores */
	sev();

	for (int i = 1; i < PLATFORM_CORE_COUNT; i++) {
		INFO("Waiting for core #%d to be up\n", i);
		while(!cpu_release_succeeded[i]);
		INFO("core #%d released\n", i);
	}
}


void hailo15_gic_my_core_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

static void hailo15_gic_init(void)
{
	gicv3_driver_init(&plat_gic_data);
	gicv3_distif_init();
	hailo15_gic_my_core_init();
}

void bl31_platform_setup(void)
{
	hailo15_gic_init();
	bl31_setup_secondary_cpus();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	return NULL;
}

uint32_t plat_get_syscnt_freq2(void)
{
	return PLAT_SYSCNT_FREQ;
}

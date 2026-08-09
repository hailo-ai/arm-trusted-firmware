#include <arch.h>
#include <asm_macros.S>
#include <cortex_a53.h>
#include <cpu_macros.S>
#include <platform_def.h>

	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	platform_mem_init
	.globl	plat_my_core_pos
	.globl	plat_secondary_cpus_bl31_entry
	.globl	hailo_secondary_pwr_down

/*
 * Detect a warm boot (system-suspend resume).
 * Warm boot can only happen on core 0, and on resume the SCU resets the core,
 * which re-enters BL31 cold and calls here. If we are on core 0 and
 * the magic matches we clear it (so the next reset is cold again)
 * and return the stored warm entrypoint; otherwise return 0 (cold boot).
 */
func plat_get_my_entrypoint
	mov	x3, x30
	bl	plat_my_core_pos
	mov	x30, x3
	cbnz	x0, 1f
	mov_imm	x1, PLAT_WARM_BOOT_INFO_ADDR
	ldr	x0, [x1]
	mov_imm	x2, PLAT_WARM_BOOT_MAGIC
	cmp	x0, x2
	b.ne	1f
	/* Warm boot: consume the marker and return the stored entrypoint. */
	str	xzr, [x1]
	ldr	x0, [x1, #8]
	ret
1:
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint


func plat_secondary_cold_boot_setup
	bl	plat_my_core_pos
	mov_imm	x1, SECONDARY_CPU_SPIN_BASE_ADDR
	add	x0, x1, x0, LSL #3

	/* Wait until the entrypoint gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
	/* consume the entry point so the next power-down cycle waits for a fresh release. */
	str	xzr, [x0]
	br	x1
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup


func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #MPIDR_CPU_MASK
	cmp	x0, #0 /* primary is #0 */
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x0, x0, #MPIDR_CPU_MASK
	ret

endfunc plat_my_core_pos

func plat_secondary_cpus_bl31_entry
	/* Signal to the primary core that U-Boot SPL
	   succesfully released this core to us */
	bl	plat_my_core_pos
	mov_imm	x1, CPU_RELEASE_SUCCESS_TABLE
	add	x0, x1, x0, LSL #3
	mov	x1, #1
	str	x1, [x0]
	/* behave as if this is a cold reset */
	b bl31_entrypoint
endfunc plat_secondary_cpus_bl31_entry

/*
 * Return a powered-down secondary core to the boot holding pen so a later
 * PSCI CPU_ON re-releases it through the warm-boot path. The power-down
 * sequence already flushed and disabled the data cache; turn off the MMU
 * and instruction cache too so the pen runs in a cold-reset context.
 * Does not return.
 */
func hailo_secondary_pwr_down
	bl	disable_mmu_icache_el3
	b	plat_secondary_cold_boot_setup
endfunc hailo_secondary_pwr_down

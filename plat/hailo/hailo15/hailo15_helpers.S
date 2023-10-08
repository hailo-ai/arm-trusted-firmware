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

func plat_get_my_entrypoint
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint


func plat_secondary_cold_boot_setup
	bl	plat_my_core_pos
	mov_imm	x1, SECONDARY_CPU_SPIN_BASE_ADDR
	add	x0, x1, x0, LSL #3
	mov	x1, #0
	str	x1, [x0]

	/* Wait until the entrypoint gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
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
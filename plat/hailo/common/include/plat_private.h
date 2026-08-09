#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

void hailo15_gic_my_core_init(void);
void hailo_gic_cpuif_disable(void);
void hailo_gic_cpuif_enable(void);
void __dead2 hailo_secondary_pwr_down(void);
void hailo_scmi_init(void);
void __dead2 hailo_scmi_sys_shutdown(void);
void __dead2 hailo_scmi_sys_reboot(void);
void hailo_scmi_sys_suspend(void);

extern volatile uint64_t * const secondary_release_addresses;

#endif /* PLAT_PRIVATE_H */

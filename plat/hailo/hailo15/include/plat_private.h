#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

void hailo15_gic_my_core_init(void);

extern volatile uint64_t * const secondary_release_addresses;

#endif /* PLAT_PRIVATE_H */

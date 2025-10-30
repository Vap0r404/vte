#ifndef VTE_PLATFORM_H
#define VTE_PLATFORM_H

/* Platform abstraction layer for cross-platform compatibility */

/* Initialize platform-specific terminal/console settings */
void platform_init(void);

/* Cleanup platform-specific resources */
void platform_cleanup(void);

#endif /* VTE_PLATFORM_H */

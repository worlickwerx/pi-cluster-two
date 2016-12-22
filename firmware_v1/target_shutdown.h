#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_shutdown_setup (void);
EXTERNC void target_shutdown_finalize (void);

EXTERNC void target_shutdown_update (void);
EXTERNC void target_shutdown_pulse (void);
EXTERNC void target_shutdown_set (uint8_t val);
EXTERNC void target_shutdown_get (uint8_t *val);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


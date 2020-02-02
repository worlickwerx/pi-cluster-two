#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_reset_setup (void);
EXTERNC void target_reset_finalize (void);

EXTERNC void target_reset_update (void);
EXTERNC void target_reset_pulse (void);
EXTERNC void target_reset_set (uint8_t val);
EXTERNC void target_reset_get (uint8_t *val);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


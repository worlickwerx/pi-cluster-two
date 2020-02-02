#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_power_setup (void);
EXTERNC void target_power_finalize (void);

EXTERNC void target_power_update (void);
EXTERNC void target_power_set (uint8_t val);
EXTERNC void target_power_get (uint8_t *val);

EXTERNC void target_power_measure (uint16_t *val); // in mA


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


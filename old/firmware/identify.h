#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void identify_setup (void);
EXTERNC void identify_finalize (void);

EXTERNC void identify_update (void);
EXTERNC void identify_set (uint8_t val);
EXTERNC void identify_get (uint8_t *val);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


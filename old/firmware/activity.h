#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void activity_setup (void);
EXTERNC void activity_finalize (void);

EXTERNC void activity_update (void);
EXTERNC void activity_pulse (void);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


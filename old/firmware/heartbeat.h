#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void heartbeat_setup (void);
EXTERNC void heartbeat_finalize (void);
EXTERNC void heartbeat_update (void);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


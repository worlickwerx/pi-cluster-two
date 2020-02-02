#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void canmgr_setup (uint8_t mod, uint8_t node);
EXTERNC void canmgr_finalize (void);
EXTERNC void canmgr_update (void);

EXTERNC void canmgr_heartbeat_send (uint8_t *data, uint8_t len);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


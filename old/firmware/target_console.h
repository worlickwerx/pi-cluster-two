#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_console_setup (void);
EXTERNC void target_console_finalize (void);
EXTERNC void target_console_update (void);

EXTERNC int target_console_available (void);
EXTERNC void target_console_reset (void);
EXTERNC void target_console_send (uint8_t *buf, int len);
EXTERNC int target_console_recv (uint8_t *buf, int len);

EXTERNC int target_console_history_next (uint8_t *buf, int len);
EXTERNC void target_console_history_reset (void);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


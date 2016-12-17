#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_console_setup (void);
EXTERNC void target_console_finalize (void);

EXTERNC int target_console_available (void);
EXTERNC void target_console_send (uint8_t *buf, int len);
EXTERNC int target_console_recv (uint8_t *buf, int len);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void (*console_receiver_f )(char *buf, int len, void *arg);

EXTERNC void target_console_setup (void);
EXTERNC void target_console_finalize (void);

EXTERNC void target_console_update (void);

// receiver callback will get 1-4 chars per call
EXTERNC void target_console_set_receiver (console_receiver_f cb, void *arg);

EXTERNC void target_console_send (char *buf, int len);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


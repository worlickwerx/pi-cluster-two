void console_setup (void);
void console_finalize (void);

void console_update (void);

int console_available (void);
void console_reset (void);
void console_send (uint8_t *buf, int len);
int console_recv (uint8_t *buf, int len);

int console_history_next (uint8_t *buf, int len);
void console_history_reset (void);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

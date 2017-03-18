void itm_putchar (uint32_t ch);
void itm_puts (const char *s);
void itm_printf (const char *fmt, ...);
void itm_fatal (const char *msg, const char *file, int line);
int itm_enabled (void);

#define FATAL(msg)   itm_fatal (msg, __FILE__, __LINE__)


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

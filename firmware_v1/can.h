#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void* can_device_t;

// begin with global mask (0 to allow everything)
EXTERNC void can0_begin (uint32_t idmask);
EXTERNC void can0_end (void);

// must set all filters if using any, n = 0...7
EXTERNC void can0_setfilter (uint32_t idmask, uint8_t n);

EXTERNC int can0_available (void);

EXTERNC int can0_write (uint32_t id, uint8_t len, uint8_t *buf,
                        uint16_t timeout);

EXTERNC int can0_read (uint32_t *id, uint8_t *len, uint8_t *buf);

#undef EXTERNC

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "canmgr_proto.h"
#include "canmgr_dump.h"

void canmgr_dump (struct canmgr_frame *fr, char *buf, int len)
{
    const char *typestr, *objstr;
    char s[16];
    char hex[32];
    char ascii[35];
    int i;

    /* stringify type */
    switch (fr->type) {
        case CANMGR_TYPE_RO:  typestr = "RO"; break;
        case CANMGR_TYPE_WO:  typestr = "WO"; break;
        case CANMGR_TYPE_WNA: typestr = "WNA"; break;
        case CANMGR_TYPE_DAT: typestr = "D"; break;
        case CANMGR_TYPE_ACK: typestr = "ACK"; break;
        case CANMGR_TYPE_NAK: typestr = "NAK"; break;
        case CANMGR_TYPE_SIG: typestr = "SIG"; break;
    }
    /* stringify object */
    switch (fr->object) {
        case CANOBJ_TARGET_CONSOLECONN: objstr = "CONSOLECONN"; break;
        case CANOBJ_TARGET_CONSOLEDISC: objstr = "CONSOLEDISC"; break;
        case CANOBJ_TARGET_CONSOLESEND: objstr = "CONSOLESEND"; break;
        case CANOBJ_TARGET_CONSOLERECV: objstr = "CONSOLERECV"; break;
        case CANOBJ_TARGET_CONSOLERING: objstr = "CONSOLERING"; break;
        case CANOBJ_TARGET_POWER:       objstr = "POWER"; break;
        case CANOBJ_TARGET_RESET:       objstr = "RESET"; break;
        case CANOBJ_TARGET_SHUTDOWN:    objstr = "SHUTDOWN"; break;
        case CANOBJ_LED_IDENTIFY:       objstr = "IDENTIFY"; break;
        default:
            if (fr->object >= CANOBJ_TARGET_CONSOLEBASE)
                snprintf (s, sizeof (s), "CONSEND/%02x",
                          fr->object - CANOBJ_TARGET_CONSOLEBASE);
            else
                snprintf (s, sizeof (s), "%03x", fr->object);
            objstr = s;
            break;
    }
    /* stringify hex data */
    hex[0] = '\0';
    for (i = 0; i < fr->dlen; i++)
        snprintf (&hex[i*3], 4, "%02x%s", fr->data[i],
                 i == fr->dlen - 1 ? "" : " ");

    /* stringify ascii data */
    memcpy (ascii, fr->data, fr->dlen);
    for (i = 0; i < fr->dlen; i++)
        if (!isprint (ascii[i]))
            ascii[i] = '.';

    snprintf (buf, len, "%.3x->%.3x  %4s %.2x,%.2x %12s %-23s %s%.*s%s",
              fr->src, fr->dst,
              typestr,
              fr->module, fr->node,
              objstr, hex,
              fr->dlen > 0 ? "`" : "",
              fr->dlen, ascii,
              fr->dlen > 0 ? "\'" : "");
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


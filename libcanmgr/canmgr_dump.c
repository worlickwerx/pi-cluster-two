#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "canmgr_proto.h"
#include "canmgr_dump.h"

void canmgr_dump (struct canmgr_frame *fr, char *buf, int len)
{
    const char *typestr, *objstr;
    char objstr_hex[5];
    char datastr[4][4];
    char dataprn[7];
    int i;

    switch (fr->hdr.type) {
        case CANMGR_TYPE_RO:  typestr = "RO"; break;
        case CANMGR_TYPE_WO:  typestr = "WO"; break;
        case CANMGR_TYPE_WNA: typestr = "WNA"; break;
        case CANMGR_TYPE_DAT: typestr = "D"; break;
        case CANMGR_TYPE_ACK: typestr = "ACK"; break;
        case CANMGR_TYPE_NAK: typestr = "NAK"; break;
        case CANMGR_TYPE_SIG: typestr = "SIG"; break;
    }
    switch (fr->hdr.object) {
        case CANOBJ_TARGET_CONSOLECONN: objstr = "CONSOLECONN"; break;
        case CANOBJ_TARGET_CONSOLEDISC: objstr = "CONSOLEDISC"; break;
        case CANOBJ_TARGET_POWER:       objstr = "POWER"; break;
        default:
            snprintf (objstr_hex, sizeof (objstr_hex), "%.4x", fr->hdr.object);
            objstr = objstr_hex;
            break;
    }
    memset (dataprn, 0, sizeof (dataprn));
    if (fr->dlen > 0)
        dataprn[0] = '`';
    for (i = 0; i < fr->dlen; i++) {
        snprintf (datastr[i], 4, "%.2x", fr->data[i]);
        dataprn[i+1] = isprint (fr->data[i]) ? fr->data[i] : '.';
    }
    if (fr->dlen > 0)
        dataprn[fr->dlen + 1] = '\'';
    snprintf (buf, len, "%.3x->%.3x  %4s %.2x,%.2x,%.2x %12s "
                         "%2s %2s %2s %2s %s",
            
              fr->id.src,
              fr->id.dst,
              typestr,
              fr->hdr.cluster,
              fr->hdr.module,
              fr->hdr.node,
              objstr,
              fr->dlen > 0 ? datastr[0] : "",
              fr->dlen > 1 ? datastr[1] : "",
              fr->dlen > 2 ? datastr[2] : "",
              fr->dlen > 3 ? datastr[3] : "",
              dataprn
            );
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */


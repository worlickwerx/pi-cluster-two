#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <stdint.h>
#include <string.h>
#include <linux/can.h>

#include "canmgr_proto.h"
#include "lxcan.h"

int lxcan_open (const char *name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
        return -1;
    strcpy(ifr.ifr_name, "can0" );
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        close (s);
        return -1;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind (s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close (s);
        return -1;
    }
    return s;
}

int lxcan_send (int s, struct canmgr_frame *fr)
{
    struct rawcan_frame raw;
    struct can_frame lin;

    if (canmgr_encode (fr, &raw) < 0)
        return -1;
    lin.can_id = raw.id;
    lin.can_dlc = raw.dlen;
    memcpy (lin.data, raw.data, raw.dlen);

    if (write (s, &lin, sizeof(lin)) != sizeof (lin))
        return -1;
    return 0;
}

int lxcan_recv (int s, struct canmgr_frame *fr)
{
    struct rawcan_frame raw;
    struct can_frame lin;

    if (read (s, &lin, sizeof(lin)) != sizeof (lin))
        return -1;

    raw.id = lin.can_id;
    raw.dlen = lin.can_dlc;
    memcpy (raw.data, lin.data, lin.can_dlc);
    if (canmgr_decode (fr, &raw) < 0)
        return -1;
    return 0;
}

void lxcan_close (int s)
{
    close (s);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

int lxcan_open (const char *name);
void lxcan_close (int s);

int lxcan_send (int s, struct canmgr_frame *fr);
int lxcan_recv (int s, struct canmgr_frame *fr);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

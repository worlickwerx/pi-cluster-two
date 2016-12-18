int lxcan_open (const char *name);
void lxcan_close (int s);

int lxcan_send (int s, struct rawcan_frame *raw);
int lxcan_recv (int s, struct rawcan_frame *raw);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

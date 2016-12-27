# install libev-dev

PROGS = canpower cancon cansnoop canaddr canping
CANMGRDIR := $(shell pwd)/../libcanmgr
CFLAGS = -I$(CANMGRDIR)
LIBADD = -L$(CANMGRDIR) -lcanmgr -lev

all: $(PROGS)

canpower: canpower.o lxcan.o address.o
	$(CC) -o $@ $^ $(LIBADD)

canping: canping.o lxcan.o address.o monotime.o
	$(CC) -o $@ $^ $(LIBADD)

cancon: cancon.o lxcan.o address.o
	$(CC) -o $@ $^ $(LIBADD)

canaddr: canaddr.o address.o
	$(CC) -o $@ $^

cansnoop: cansnoop.o lxcan.o monotime.o
	$(CC) -o $@ $^ $(LIBADD)

clean:
	rm -f $(PROGS)
	rm -f *.o

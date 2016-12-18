PROGS = canpower
CANMGRDIR := $(shell pwd)/../libcanmgr
CFLAGS = -I$(CANMGRDIR)
LIBADD = -L$(CANMGRDIR) -lcanmgr

all: $(PROGS)

canpower: canpower.o
	$(CC) -o $@ $< $(LIBADD)

clean:
	rm -f $(PROGS)
	rm -f *.o

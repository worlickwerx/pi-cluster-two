OBJS = canmgr_proto_libcanmgr.o canmgr_dump_libcanmgr.o

all: libcanmgr.a

libcanmgr.a: $(OBJS)
	$(AR) rv $@ $^

%_libcanmgr.o:%.c
	$(CC) -c -o$@ $<

clean:
	rm -f libcanmgr.a
	rm -f *.o

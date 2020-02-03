.PHONY = libopencm3 clobber_libopencm3 clean_libopencm3 src
.PHONY += stlink clean_stlink clobber_stlink

all: libopencm3 stlink src
	$(MAKE) -$(MAKEFLAGS) -C ./src

clean: clean_libopencm3 clean_stlink
	$(MAKE) -$(MAKEFLAGS) -C ./src clean

clobber: clobber_libopencm3 clobber_stlink
	$(MAKE) -$(MAKEFLAGS) -C ./src clobber

clean_libopencm3: clobber_libopencm3

clobber_libopencm3:
	rm -f libopencm3/lib/libopencm3_stm32f1.a
	-$(MAKE) -$(MAKEFLAGS) -C ./libopencm3 clean

libopencm3: libopencm3/lib/libopencm3_stm32f1.a

libopencm3/lib/libopencm3_stm32f1.a:
	$(MAKE) -C libopencm3 TARGETS=stm32/f1

stlink: stlink/build/Project/st-flash

stlink/build/Project/st-flash:
	$(MAKE) -C stlink

clean_stlink:
	$(MAKE) -C stlink clean

clobber_stlink: clean_stlink

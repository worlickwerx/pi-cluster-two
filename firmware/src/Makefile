######################################################################
#  Project Makefile
######################################################################

BINARY		= main
SRCFILES	= main.c \
		  power.c \
		  address.c \
		  blink.c \
		  matrix.c \
		  trace.c \
		  canbus.c \
		  canmsg_v1.c \
		  rtos/heap_4.c \
		  rtos/list.c \
		  rtos/port.c \
		  rtos/queue.c \
		  rtos/tasks.c \
		  rtos/opencm3.c \
		  rtos/stream_buffer.c \
		  libwwg/miniprintf.c
LDSCRIPT	= stm32f103c8t6.ld
CFLAGS		+= -Ilibwwg

# DEPS		= 	# Any additional dependencies for your build
# CLOBBER	+= 	# Any additional files to be removed with "make clobber"

include ../Makefile.incl
include ../Makefile.rtos

######################################################################
#  NOTES:
#	1. remove any modules you don't need from SRCFILES
#	2. "make clean" will remove *.o etc., but leaves *.elf, *.bin
#	3. "make clobber" will "clean" and remove *.elf, *.bin etc.
#	4. "make flash" will perform:
#	   st-flash write main.bin 0x8000000
######################################################################

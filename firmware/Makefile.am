.NOTPARALLEL:

AM_CPPFLAGS = \
    -I$(srcdir) \
    -I$(srcdir)/.. \
    -I$(srcdir)/libopencm3/include \
    -DSTM32F1 \
    -MD

warnflags = \
    -Wall \
    -Wextra \
    -Wshadow \
    -Wundef \
    -Werror \
    -Wredundant-decls \
    -Wmissing-prototypes \
    -Wstrict-prototypes

AM_CFLAGS = \
    -std=c99 \
    -mthumb \
    -mcpu=cortex-m3 \
    -msoft-float \
    -mfix-cortex-m3-ldrd \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    $(warnflags)

AM_LDFLAGS = \
    --static \
    -nostartfiles \
    -T$(srcdir)/libwwg/stm32f103c8t6.ld \
    -Wl,-Map=bramble.map \
    -Wl,--gc-sections \
    -specs=nosys.specs \
    -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

BUILT_SOURCES = libopencm3/stm32/f1

EXEEXT = .fw

firmwaredir = /lib/firmware

firmware_PROGRAMS = bramble

noinst_LIBRARIES = \
    librtos/librtos.a

bramble_SOURCES = \
    src/main.c \
    src/power.c \
    src/power.h \
    src/address.c \
    src/address.h \
    src/blink.c \
    src/blink.h \
    src/matrix.c \
    src/matrix.h \
    src/trace.c \
    src/trace.h \
    src/canbus.c \
    src/canbus.h \
    src/canservices.c \
    src/canservices.h \
    src/serial.c \
    src/serial.h \
    src/i2c.c \
    src/i2c.h \
    src/rtc.c \
    src/rtc.h \
    src/font5x7.h \
    libwwg/miniprintf.c \
    libwwg/miniprintf.h \
    libwwg/opencm3.c

bramble_LDADD = \
    librtos/librtos.a \
    libopencm3/lib/libopencm3_stm32f1.a

librtos_librtos_a_SOURCES = \
    librtos/FreeRTOSConfig.h \
    librtos/FreeRTOS.h \
    librtos/heap_4.c \
    librtos/list.c \
    librtos/port.c \
    librtos/queue.c \
    librtos/tasks.c \
    librtos/stream_buffer.c \
    librtos/croutine.h \
    librtos/deprecated_definitions.h \
    librtos/event_groups.h \
    librtos/list.h \
    librtos/mpu_prototypes.h \
    librtos/mpu_wrappers.h \
    librtos/portable.h \
    librtos/portmacro.h \
    librtos/projdefs.h \
    librtos/queue.h \
    librtos/semphr.h \
    librtos/stack_macros.h \
    librtos/StackMacros.h \
    librtos/stream_buffer.h \
    librtos/task.h \
    librtos/timers.h
librtos_librtos_a_CPPFLAGS = \
    $(AM_CPPFLAGS) \
    -I$(srcdir)/librtos

all-local:
	$(SIZE) bramble.fw

# libopencm3 has its own separate build system
# it's a git submodule so we don't change anything in there
libopencm3/lib/libopencm3_stm32f1.a: libopencm3/stm32/f1
libopencm3/stm32/f1:
	$(MAKE) -C libopencm3 TARGETS=stm32/f1
clean-local:
	$(MAKE) -C libopencm3 clean TARGETS=stm32/f1

EXTRA_DIST = \
    librtos/heap_1.c \
    librtos/heap_2.c \
    librtos/heap_3.c \
    librtos/heap_5.c \
    librtos/LICENSE \
    librtos/stdint.readme \
    libopencm3 \
    libwwg/stm32f103c8t6.ld

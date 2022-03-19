# Building and developing firmware with pi as host

## Prerequisites

The following packages need to be installed on raspbian to support
building and flashing the stm32f1 firmware.  The os versions of these
packages work fine, at least on rasbian bullseye.
```
$ sudo apt update
$ sudo apt install openocd gcc-arm-none-eabi gdb-multiarch
```

For the record, the versions that are working now are:
- openocd 0.11.0~rc2-1
- gdb-multiarch 10.1-1.7
- gcc-arm-none-eabi 15:8-2019-q3-1+13

## Submodules

libopencm3 is set up as a submodule of this project.  It should be present
if this project was checked out with:
```
$ git clone --recurse-submodules uri
```
If not, then run:
```
$ git submodule init
$ git submodule update
```

## Building the firmware

With submodules and prereqs in place, the firmware should build with:
```
$ cd firmware
$ make
```

## Flashing the firmware

The default config assumes the pi has a hardwired gpio SWD connection.
Edit `src/Makefile` if you want to use the stlink v2 instead:
```diff
diff --git a/firmware/src/Makefile b/firmware/src/Makefile
index a5594cb..14e4ce2 100644
--- a/firmware/src/Makefile
+++ b/firmware/src/Makefile
@@ -44,8 +44,8 @@ include ../Makefile.rtos
 ######################################################################

 OPENOCD = openocd
-OPENOCD_CFG = openocd_pi.cfg
-#OPENOCD_CFG = openocd_stlink.cfg
+#OPENOCD_CFG = openocd_pi.cfg
+OPENOCD_CFG = openocd_stlink.cfg

 flash:
        sudo $(OPENOCD) -f $(OPENOCD_CFG) \

```

To flash:
```
$ make flash
```

To reset
```
$ make reset
```

## Remote debugging with gdb

In one window type:
```
$ make gdbserver
```

In another, type:
```
$ make debug
```

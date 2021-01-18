OVERLAYS = \
	mcp2515-can2.dtbo

BOOT_DT_DIR = /boot/overlays

all: $(OVERLAYS)

%.dtbo: %-overlay.dts
	dtc -@ -I dts -O dtb -o $@ $<

install: $(OVERLAYS)
	cp $^ $(BOOT_DT_DIR)/

uninstalsl:
	cd $(BOOT_DT_DIR) && rm -f $(OVERLAYS)

clean:
	rm -f $(OVERLAYS)

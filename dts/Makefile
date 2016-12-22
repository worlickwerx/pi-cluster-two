# install shutdown.dtbo in /boot/overlays
# add "dtoverlay=shutdown" to config.txt
# install 70-power-switch.rules to /etc/udev/rules.d
# comment out "HandlePowerKey=poweroff" in /etc/systemd/logind.conf
# reboot

all: shutdown.dtbo

shutdown.dtbo: shutdown.dts
	dtc -@ -I dts -O dtb -o $@ $<

install:
	cp $< /boot/overlays/
	cp 70-power-switch.rules /etc/udev/rules.d/

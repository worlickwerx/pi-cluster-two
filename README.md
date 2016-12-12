## pi-slavehat

pi-slavehat is a Pi HAT for remote control of some aspects of a
Raspberry Pi that become very convenient in clusters of more than a
dozen or so nodes:

* Power
* Reset
* Serial console
* Remote JTAG for GDB/OpenOCD
* Boot mode selection
* Power/Temp monitoring

This is accomplished over a management CAN bus, which has been used before
for this purpose on the [Meiko CS/2](https://github.com/garlick/meiko-cs2).

While the goal is to make a compact HAT for remote management,
The initial prototype is actually spread out on a 3U eurocard which has
a Raspberry Pi 3 on standoffs, and some other hardware including
a [teensy3.2](https://www.pjrc.com/store/teensy32.html) as a management
controller.  The teensy is attached to the CAN bus via a
[MCP2551 CAN transceiver](http://www.microchip.com/wwwproducts/en/en010405).
The teensy can pull down the Pi reset line with one of its GPIO's.
It can also control the Pi power using a
[TPS22958NDGK load switch](http://www.ti.com/product/TPS22958).
Finally, one of the teensy's serial ports is attached to the Pi console
serial port.  TBD is to connect Pi GPIO's for boot mode election and
remote JTAG, and to attach current and temperature sensors.

The Pi itself gets a CAN interface through an
[MCP2515 SPI CAN controller](http://www.microchip.com/wwwproducts/en/en010406).

The Eurocard plugs into a DIN backplane which supplies power and the CAN
bus.

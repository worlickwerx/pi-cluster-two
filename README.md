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
* Beacon LED

This is accomplished over a management CAN bus, which has been used before
for this purpose on the [Meiko CS/2](https://github.com/garlick/meiko-cs2).

### v1 Prototype

While the goal is to make a compact HAT for remote management,
the initial prototype is actually spread out on a 3U eurocard which has
a Raspberry Pi 3 on standoffs, and some other hardware including
a [teensy3.2](https://www.pjrc.com/store/teensy32.html) as a management
controller.  The teensy is attached to the CAN bus via a
[MCP2551 CAN transceiver](http://www.microchip.com/wwwproducts/en/en010405).
The teensy can
* pull down the Pi reset line with one of its GPIO's
* control the Pi power using a [TPS22958 load switch](http://www.ti.com/product/TPS22958)
* flash a beacon LED to identify a board that needs service
* read/write the Pi console serial port
* drive GPIO boot mode selection pins (TBD)
* control Pi JTAG pins (TBD)
* monitor Pi current and temperature (TBD)
* set unique CAN address from DIP switch/slot ID (TBD)

The Pi itself also gets a CAN interface through a
[MCP2515 SPI CAN controller](http://www.microchip.com/wwwproducts/en/en010406).

The Eurocard plugs into a DIN backplane which supplies power and interconnects
the CAN endpoints.

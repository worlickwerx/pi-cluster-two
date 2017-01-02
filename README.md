## pi-compute-board

pi-compute-board is a Pi add-on hardware for remote control of some aspects
of a Raspberry Pi that become very convenient in clusters of more than a
dozen or so nodes:

* Power
* Reset
* Serial console
* Power monitoring
* Beacon LED

This is accomplished over a management CAN bus, similar to that used
on the [Meiko CS/2](https://github.com/garlick/meiko-cs2).

### v1 Prototype

The initial prototype is a 3U eurocard with the Pi 3 on standoffs, and
some other hardware including a
[teensy3.2](https://www.pjrc.com/store/teensy32.html) as a management
controller.  The teensy is attached to the L-CAN bus via a
[MCP2551 CAN transceiver](http://www.microchip.com/wwwproducts/en/en010405).
The teensy can
* pull down the Pi reset line with one of its GPIO's
* hard power on/off the Pi using a [TPS22958 load switch](http://www.ti.com/product/TPS22958)
* soft power off the Pi using systemd's POWER\_OFF button
* monitor for shutdown completion during soft power off
* flash a beacon LED to identify a board that needs service
* read/write the Pi console serial port and buffer 32K of history
* monitor Pi external current draw
* read L-CAN address from DIP switch
* read X-CAN address from DIN backplane.
* answer CAN queries

The Pi itself also gets a L-CAN interface through a
[MCP2515 SPI CAN controller](http://www.microchip.com/wwwproducts/en/en010406)
and reads the L-CAN and X-CAN addresses via GPIO.

The Eurocard plugs into a DIN backplane which supplies power and interconnects
L-CAN endpoints.

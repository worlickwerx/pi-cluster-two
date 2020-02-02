## pi-compute-board

pi-compute-board is a 3U molleibus carrier for Raspberry Pi intended for
ganging up multiple Pi's in a molliebus crate for parallel computation.
The Pi is monitored and controlled with a _service processor_, embedded
on each carrier board.

### service processor

The service processor is based on the STM32F1 ARM Cortex-M3 microprocessor
running FreeRTOS.  It communicates with the molliebus crate controller
over the CAN control bus, and with the Pi over I2C.  It offers the following
functions:

* Remote CAN power on/off of Pi 5V supply
* Remote CAN current monitoring of the Pi 5V supply
* Remote CAN temperature sensing
* Remote CAN reset of the Pi
* Remote CAN _soft shutdown_ of the Pi
* Remote CAN and I2C control of a 5x7 LED matrix
* Remote CAN access to the Pi console serial port

### molliebus

Molliebus is a (work in progress) bus design utilizing Eurocard 3U
and 6U (160mm depth) boards with DIN 41612 connectors.  It can coexist
with VME, utilizing rows a and c of the P2 connector, or be implemented
as a standalone bus.  The "bus" portion thus far
* distributes power (3V3, 5V, 12V, and 5V-standby rails)
* provides a crate-wide reset signal
* implements a 1 mbps CAN control bus
* provides _geographic slot addressing_
* specifes a slot 1 "crate controller" role

There can be up to 31 slots per crate, including the crate controller.

Half of the available pins (row c, pins 1-32) are left to be user-defined,
with the idea that high speed communication between boards will require
point to point or switched differential pair signaling (such as USB or
ethernet).  The design of these interconnects can be application specific,
and need not be specified as part of the bus design.

Row c is not used by the Pi carrier, as the Pi already has appropriately
terminated USB and ethernet connectors, and these are made available via
the Pi carrier faceplate.

### origin and license

This project includes parts of:
* FreeRTOS (MIT license)
* libopencm3 (LGPLv3+)
* project templates from the book _Beginning STM32_ by Warren Gay (LGPLv3+)

The CAN control bus and LED matrix were influenced by similar features
of the [Meiko CS/2](https://github.com/garlick/meiko-cs2), a ground breaking
supercomputer of the 1990's.

#### software

SPDX-License-Identifier: GPL-3.0-or-later

#### hardware

SPDX-License-Identifier: CERN-OHL-1.2

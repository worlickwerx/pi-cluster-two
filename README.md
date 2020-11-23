## pi-cluster-two

This project is a redesign of
[pi-cluster-one](https://github.com/garlick/pi-cluster-one).

The planned scale of this design is 16 nodes, and its primary intended use
is as a vehicle for learning about HPC cluster internals, hardware design,
and the open source ecosystem for hardware design.

This cluster is implemented on 100 x 160mm Eurocards plugged into a 16-slot
custom backplane with 96-pin DIN 41612 connectors.

A compute board carries a Raspberry Pi 4B with support circuitry including
a service processor for remote management and monitoring over CAN bus,
a 5x7 LED bargraph for status display, a CAN adapter for the Pi, and a
mezzanine slot for a communications adapter.

The backplane supports power distribution, geographic slot addressing,
the CAN mangement/monitoring bus, a wired-OR hardware barrier network,
a tree network for optimized MPI collectives, and a 2D torus mesh network.
Since the backplane connector is not optimized for differential signaling,
serial channels for these networks will operate at a few megabits per second.

### status Nov 2020

The [second version](hardware/pi-carrier/README.md) of the compute board
PCB was just sent out for fabrication.  Three boards of the first version
are functional with the CAN management software from pi-cluster-one.

Firmware TODO: remote serial console, I2C slave.

Hardware TODO:  design full 16-slot backplane, design comms mezzanine card.

### origin and license

This project includes parts of:
* FreeRTOS (MIT license)
* libopencm3 (LGPLv3+)
* project templates from the book _Beginning STM32_ by Warren Gay (LGPLv3+)

The CAN control bus and LED matrix were influenced by similar features
of the [Meiko CS/2](https://github.com/garlick/meiko-cs2), a ground breaking
supercomputer of the 1990's.

The communcations design is patterned after
[Blue Gene/L](https://en.wikipedia.org/wiki/IBM_Blue_Gene), circa 2005.

#### software

SPDX-License-Identifier: GPL-3.0-or-later

#### hardware

SPDX-License-Identifier: CERN-OHL-1.2

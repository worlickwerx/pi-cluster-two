## pi-cluster-two

This project is a redesign of
[pi-cluster-one](https://github.com/garlick/pi-cluster-one).

The planned scale of this design is 16 nodes, with a stretch goal of 64.

Its primary intended use is as a vehicle for learning about HPC cluster
internals, hardware design, and the open source ecosystem for hardware design.

The building blocks that comprise this cluster are:
* An 8-slot backplane with 96-pin DIN 41612 connectors
* A backplane supervisor (one per one or two 8-slot backplanes)
* An ATX (20-pin) power supply
* A raspberry pi compute node on a 100 x 160mm Eurocard
* A communications adapter, fitted on the compute node as a mezzanine board

### Networking

In addition to a regular gigabit ethernet network, the design of this
cluster supports:
* a CAN network for system management (on backplane)
* A wired-OR network for fast hardware barriers
* A binary tree for broadcast/reduction, consisting of three signle-lane,
  bidirectional LVDS links
* A 3D torus, consisting of six single-lane, bidirectional LVDS links
  (4 used for 2D torus, 2 for 1D torus)

The left-right torus dimension is hardwired on the backplane, in two
groups of four nodes.  Remaining torus and tree network links require
2-pair UTP cables connected to JST connectors on the backplane.

### System configurations

**compute nodes** | **backplanes** | **supervisors** | **tree cables** | **torus cables** | **notes**
:-- | :-- | :-- | :-- | :-- | :--
4   | 1   | 1   | 3   | 0   | 1D torus
16  | 2   | 1   | 15  | 16  | 2D torus (4x4)
64  | 8   | 4   | 63  | 128 | 3D torus (4x4x4)

### Compute node

A compute board carries a Raspberry Pi 4B with support circuitry including
a service processor for remote management and monitoring over CAN bus,
a 5x7 LED bargraph for status display, a CAN adapter for the Pi, and a
mezzanine slot for a communications adapter.

10 [version 2](hardware/pi-carrier/README.md) compute board PCBs were
were received and are being hand assembled/debugged.

A CM4 version is planned.

### Backplane

The backplane supports power distribution, geographic slot addressing,
a CAN mangement/monitoring bus, a wired-OR hardware barrier network,
a tree network for optimized MPI collectives, and a 2D torus mesh network.

Five [version 1](hardware/bus-8/README.md) backplanes have been ordered,
with PCBA assembly.

### Supervisor

The [version 1](hardware/supervisor/README.md) supervisor design is being
finalized.

### Communications adapter

Not yet designed, but availability of a PCIe lane on the CM4 makes this
somewhat more interesting than before.

### Software status Feb 2021

STM32 firmware on the compute board version 1 and 2 is fully functional.
The firmware "personality" for the supervisor board is not yet complete
(it will use the same firmware image).

Tools on linux for snooping the CAN bus with message decoding, pinging
over CAN, reading slot address over I2C, etc. are functional.

Integration with [powerman](http://github.com/chaos/powerman) and
[conman](http://github.com/dun/conman) for remote power and console
management of a single crate is functional.

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

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

In addition to a regular gigabit ethernet network, the cluster implements:
* a CAN network for system management (on backplane)
* A wired-OR network for fast hardware barriers
* A binary tree for broadcast/reduction, consisting of three signle-lane,
  bidirectional LVDS links
* A 3D torus, consisting of six single-lane, bidirectional LVDS links

The left-right torus dimension is hardwired on the backplane, in two
groups of four nodes.  Remaining torus and tree network links require
2-pair UTP cables connected to JST connectors on the backplane.

### System configurations

#### 4 Nodes

* 1 backplane
* 1 supervisor
* 4 compute nodes
* 4 comms adapters (4 of 6 torus ports unpopulated)
* 3 UTP cables for tree network

#### 8 Nodes (2 x 4 torus)

* 1 backplanes
* 1 supervisor
* 8 compute nodes
* 8 comms adapters (2 of 6 torus ports unpopulated)
* 7 UTP cables for tree network
* 4 UTP cables for 2D torus (up-down)

#### 16 Nodes (4 x 4 torus)

* 2 backplanes
* 1 supervisor
* 16 compute nodes
* 16 comms adapters (2 of 6 torus ports unpopulated)
* 15 UTP cables for tree network
* 16 UTP cables for 2D torus (up-down)

#### 64 nodes (4 x 4 x 4 torus)

* 8 backplanes
* 4 supervisor
* 64 compute nodes
* 64 comms adapters (fully populated)
* 63 UTP cables for tree network
* 128 UTP cables for 3D torus (64 up-down, 64 in-out)

### Compute node

A compute board carries a Raspberry Pi 4B with support circuitry including
a service processor for remote management and monitoring over CAN bus,
a 5x7 LED bargraph for status display, a CAN adapter for the Pi, and a
mezzanine slot for a communications adapter.

### Backplane

The backplane supports power distribution, geographic slot addressing,
the CAN mangement/monitoring bus, a wired-OR hardware barrier network,
a tree network for optimized MPI collectives, and a 2D torus mesh network.

### status Dec 2020

The [second version](hardware/pi-carrier/README.md) of the compute board
PCB was were received from pcbway and are being assembled.  Three boards
of the first version are functional with the CAN management software from
pi-cluster-one.  A compute node that uses the Pi CM4 is in design.

Five [backplanes](hardware/bus-8/README.md) were just commissioned for
assembly at pcbway.

The [supervisor board](hardware/supervisor) design was just completed.

The communications adapter is not yet designed. This would ideally be an
FPGA based design that speaks to the compute node via PCIe, however a first
cut might just be a high-end STM32 using serial with LVDS drivers, attached
to SPI, with links operating at around 3mb/s.

Firmware TODO: remote serial console, I2C slave, modify the CAN protocol
for the new architecture.

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

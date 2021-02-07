## CAN Bus

The CAN bus is used for centralized monitoring and control of power,
serial console, fans, etc..

### Physical

ISO 11898-2 defines the physical layer for the CAN bus used in this
project.

The bus runs at the maximum CAN rate of 1 mbps.

29-bit IDs are used exclusively.

CAN-FD is not used.

### Interconnection

Each crate has one local CAN bus that connects up to 16 backplane slots,
one optional crate supervisor, and one optional interface on a centralized
management node.  Each backplane slot may have up to three CAN devices
on-board, attached to the same bus.

Since the CAN busses in each crate are independent, a centralized management
node for a multi-crate cluster must have one CAN interface per crate.

### Protocol

Off-the-shelf automotive CAN protocols such as CANOpen or J1939 were
eschewed in favor of a simpler, purpose-built protocol, loosely based
on the one used in the Meiko CS/2 supercomputer.

The protocol supports:

1) unique addresses within a crate
2) _objects_ that may be read or written with request/response messages
3) _signals_, a broadcast message that indicates an object at a particular
address requires attention
5) streaming data over a stateful point to point connection

#### Headers

All messages use 29-bit CAN IDs.  Devices install CAN filters so that
they receive messages with destination address matching their own,
or the broadcast address.

The 29-bit CAN ID is split as follows:

```
+-----------------------------------------------------------+
| pri:1 | dst:6 | src:6 | type:3 | object:8 | eot:1 | seq:4 |
+-----------------------------------------------------------+
```
**pri** (0-1) is the message priority.  Since CAN bus arbitration on
collision is "lowest ID wins", the most significant bit in the ID is
a priority bit that may be set to zero for high priority messages.

**dst**, **src** (0-3f) are 6-bit device addresses uniquely identifying
the receiving and sending CAN device (see below).

**type** (0-7) is the message type:
* `RO` (0) read object
* `WO` (1) write object, with acknowledgement
* `WNA` (2) write object, without acknowledgement
* `DAT` (3) streaming data
* `ACK` (4) acknowledgement
* `NAK` (6) negative acknowledgement
* `SIG` (7) signal

**object** (0-ff) is the target object ID.

**eot** (0-1) is a flag for streaming DAT messages indicating "end of
transmission".  It should be set to 0 for other message types.

**seq** (0-f) is a message sequence number for streaming data.

#### Addresses

The 6-bit addresses are divided as follows:

```
+-------------------+
| device:2 | slot:4 |
+-------------------+
```

**device** (0-3) selects between control processor (1), compute element (2),
and communications processor (3) on a board occupying a single slot.
Device 0 is reserved for non-slot addresses (see table below).

**slot** (0-f) selects the backplane slot.

CAN addresses are most conveniently expressed as a two digit (zero padded)
hexadecimal number.

The address space is summarized as follows:

| Function           | Address |
| --------           | ------- |
| invalid address    | 00 |
| supervisor board   | 01 |
| management node    | 02 |
| broadcast          | 0f |
| control processors | 10,11,12,13,14,15,16,17,18,19,1a,1b,1c,1d,10,1f |
| compute element    | 20,21,22,23,24,25,26,27,28,29,2a,2b,2c,2d,20,2f |
| comms processor    | 30,31,32,33,34,35,36,37,38,39,3a,3b,3c,3d,30,3f |

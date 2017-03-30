### power control

Raspberry Pi power can be controlled remotely via simple CAN requests.

#### Switching 5V power

The management controller controls a high side load switch with
a GPIO output.  Initially I used a
[TPS22958](http://www.ti.com/product/TPS22958)
but later switched to a [TPS27081A](http://www.ti.com/product/TPS27081A)
because of the smaller footprint.

The load switch output is applied directly to the Pi's 5V rail via P1.

CAN commands are available for hard power on and hard power off.

#### Soft shutdown

It seems quite easy to corrupt sdcard file systems by turning off
the Pi without shutting down the OS.  On a traditional PC, a brief
press of the power button initiates a clean OS shutdown.  We can
achieve this too by simulating the power key with a Pi GPIO input
wired to a management controller GPIO output.

On the Pi, a custom device tree overlay sets up the GPIO as an
input and associates it with the
[gpio-keys](https://www.kernel.org/doc/Documentation/devicetree/bindings/input/gpio-keys.txt)
device driver.  A state transition on the GPIO input can then generate
a keypress event for key 116 (the power key) in the Linux input subsystem,
as discussed in this Raspberry Pi Forum
[thread](https://www.raspberrypi.org/forums/viewtopic.php?f=107&t=115394).

A custom udev rule is added that maps the input event for that key to
the "power-switch" tag.  Systemd then gets ahold of the event and can initiate
the shutdown, and finally take the action configured in logind.conf according
to its HandlePowerKey setting, which defaults to "poweroff".

After the shutdown completes, the Pi will enter the poweroff state;
however, the load switch is still on with the Pi CPU spinning in a 
halted state.

#### Hard Shutdown

Another Pi GPIO output pin, wired to a management controller
GPIO input, indicates when the Pi has entered the "poweroff"
state.  A device tree overlay called "gpio-poweroff" that ensures that
such an output is driven low while the OS is running and high when the
OS has halted is provided by Raspbian.

Since soft shutdown conceivably could hang, the management controller starts
a timer when shutdown is initiated.  If the Pi successfully halts
before the timer expires, the controller hard powers it off as soon as
that is detected.  Otherwise it is powered off when the timer expires.
The timer ensures that a soft power off command is as decisive as a
hard power off, within the timeout window.

#### Manual Power Control

Management controllers report node power status to the compute module
controller via CAN heartbeats.  A compute module controller may be
fitted with a rotary encoder that can select from the compute nodes that
are actively heartbeating, and toggle power state with a button press.

The "identify" CAN object is used to indicate visually which compute
node is selected as the encoder knob is rotated.  An RGB LED on the
compute module controller indicates the power status
(R=on, G=off, B=shutting down) of the currently selected node.

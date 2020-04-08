LIN Master Node Emulation
=========================

This library implements a Local Interconnect Master node emulation. For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network.

Supported functionality:
  - [blocking and non-blocking operation](../../wiki/Operation-Modes)
  - [multiple, simultaneous LIN nodes](../../wiki/Multiple-LIN) with independent baudrates and protocol
  - optional callback functions for slave response frames
  
This library depends on the *Task Scheduler* library for background operation, which is available via the [Arduino IDE library manager](../../wiki/Library-Manager.md) or directly from https://github.com/kcl93/Tasks

Supported Boards (with additional LIN hardware):
  - all boards using the Atmel ATMega328 controller, e.g. Arduino Uno and Nano
  - all boards using the Atmel ATMega2560 controller, e.g. Arduino Mega
  - all boards using the Atmel SAM3X8E controller, e.g. Arduino Due
  
As no device specific tricks are used, other boards should also work, if they are supported by the *Task Scheduler* library.

Have fun!, Georg

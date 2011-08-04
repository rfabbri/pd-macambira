OpenSoundControl (OSC) for Pd
=============================

a collection of Pd objectclasses for OSC-messages.
these objects only convert between Pd-messages and OSC-messages (binary format),
so you will need a separate set of objects that implement the transport
(OSI-Layer 4), for instance [udpsend]/[udpreceive] for sending OSC over UDP.

Author: Martin Peach

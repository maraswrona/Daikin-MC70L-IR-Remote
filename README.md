# Daikin ARC458A7 IR Remote Arduino Library (used for Daikin MC70L)

This is simple Arduino library implementing Daikin MC70L Air Purifier IR communication protocol.

I forked this code from https://github.com/ranma1988/daikin_ARC458A7_IRremote and I'm very grateful for his work as it helped me a lot to understand the communication protocol used by the remote.

## Receiver
The original library Receiver / Decoder module was using a lot of memory (long[1000] array) so it wouldn't run on my Arduino Nano, so I deciced to optimize it.

This version Receiver is completely redesigned, it implements finite state machine to read and parse IR signals and it uses only byte[16] buffer + couple of variables. So it fits perfectly on my Nano.

## Sender
Sender is almost identical to original implementation. It is simple and ligthweight to use. 

## Examples
Take a look at attached example on how to use the library.
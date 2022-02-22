# J-Box
J-Box is an open source data logger, dashboard, and instrumentation device. It is powered by a Teensy 3.6 and a 240 x 128 Graphic Touchscreen Display that interfaces with the Sparkfun NEO-M8U GPS Module for Speed, Position, and Acceleration data.

It also has 8 analog inputs (0-5v) that are protected by a replaceable buffer board. This means you can feed any voltage (up to 32v rated) to the analog inputs without frying the microcontroller or the buffer chip. Two analog inputs have provisions for configurable pull up resistors for use with thermistors. Additionally there are two clamped digital inputs that can be used as a tachometer input and a backlight signal.

A telephone jack serves as a connector to the I2C bus, allowing you to daisy chain various sensors and digital devices to the (Jack) Bus using cheap phone cables and splitters. The I2C protocol seems to be reliable over 2m provided you use shielded or twisted cables.

The primary connector (26 Position) is a MQS (Micro Quadlok) style, it uses a 0.1" pitch so you can use dupont jumpers to interface with it, or opt for the MQS system for a more durable connection.

There is also a protoboard area for you to construct your own circuits on J-Box

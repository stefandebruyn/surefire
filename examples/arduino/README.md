# Arduino Example

This example uses [Arduino CMake Toolchain](https://github.com/a9183756-gh/Arduino-CMake-Toolchain) (included in the repo root as a submodule) to cross-compile for Arduino.

To build:

```bash
cd sfa
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../Arduino-CMake-Toolchain/Arduino-toolchain.cmake
```

Note the two CMake variables. The first variable specifies the Arduino toolchain file. The second variable specifies the MAC address of the target Arduino, since this example uses sockets. If you want to actually run this code yourself you'll need to update the MAC address.

If all goes well, the last command **will fail** because we have not told the toolchain which Arduino board to build for. It will generate a file `BoardOptions.cmake`: find your Arduino board in this file and uncomment that line (e.g., for the Mega 2560, uncomment the line `set(ARDUINO_BOARD "Arduino Mega...`).

Now, rerun the `cmake` command and build the target:

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../Arduino-CMake-Toolchain/Arduino-toolchain.cmake
make arduino-example
```

To upload the binary, connect the Arduino to your computer and run:

```bash
make upload-arduino-example SERIAL_PORT=/dev/ttyUSB0
```

Change the serial port as necessary (the correct serial port can be determined in Arduino IDE by going to Tools > Port).
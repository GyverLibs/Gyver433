# Gyver433
Library for 433 MHz radio modules and Arduino
- Support for curved Chinese modules
- Built-in CRC integrity check
- Accelerated IO algorithm for AVR Arduino
- Asynchronous receive in interrupt
- Super light liba, it will start even on a tini13

### Compatibility
Compatible with all Arduino platforms (using Arduino functions)
- When connecting an interrupt on esp8266, do not forget the `IRAM_ATTR` attribute

## Content
- [Install](#install)
- [Interface](#interface)
- [Connection](#wiring)
- [Initialization](#init)
- [Usage](#usage)
- [Example](#example)
- [Versions](#versions)
- [Bugs and feedback](#feedback)

<a id="install"></a>
## Installation
- The library can be found under the name **Gyver433** and installed via the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download library](https://github.com/GyverLibs/Gyver433/archive/refs/heads/main.zip) .zip archive for manual installation:
    - Unzip and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unzip and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/*
    - (Arduino IDE) automatic installation from .zip: *Sketch/Include library/Add .ZIP library…* and specify the downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0 %BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

<a id="interface"></a>
## Interface
- Before sending data there is a clock pulse, *TRAIN* ms
- Next start impulse, *START* ms, LOW
- Data is encoded by pulse length
- Terminates the sending of a stop pulse, *TRAIN* ms
![scheme](/doc/interface.png)
![scheme](/doc/timing.png)

<a id="wiring"></a>
## Connection
![scheme](/doc/radio433.png)
Transmitters:
- SYN115, small chip: 1.8-3.6V, max. speed 8000
- FS1000A: 3-12V, max. speed 10000
- WL102-341: 2.0-3.6V, max. speed 6000

Receivers:
- SYN480R, large chip: 3.3-5.5V
- MX-RM-5V (RF-5V): 5V
- RX470 (WL101-341): 3-5V

![scheme](/doc/scheme.jpg)
![scheme](/doc/radio.png)

<a id="init"></a>
## Initialization
```cpp
// === TRANSMITTER ===
Gyver433_TX<pin> tx;
Gyver433_TX<pin, CRC> tx;

// === RECEIVER ===
Gyver433_RX<pin> rx;
Gyver433_RX<pin, buffer> rx;
Gyver433_RX<pin, buffer, CRC> rx;

// pin: digital pin
// buffer: buffer size in bytes, default 64
// CRC: data integrity check: G433_CRC8 (reliable), G433_XOR (light), G433_NOCRC (disabled). By default G433_CRC8
```

<a id="usage"></a>
## Usage
```cpp
// ========= Gyver433_TX =========
void sendData(T&data); // send data of any type (CRC added automatically)
void write(uint8_t* buf, uint8_t size); // send an array of bytes of the specified size (no CRC added)

// ========= Gyver433_RX =========
// receive in interrupt
void tickISR(); // receive ticker to call in CHANGE interrupt
uint8_t tickISRraw(); // manual reception in CHANGE interrupt. Returns 1 (beginning of reception), 2 (byte received), 3 (end of packet)

// receive in loop
uint16_t tick(); // non-blocking receive. Returns the number of successfully received bytes
uint16_t tickWait(); // blocking receive. Returns the number of successfully received bytes

bool readData(T&data); // read the buffer into any data type (into the specified penexchange)
uint16_t getSize(); // get the size of the received data
uint16_t gotData(); // will return the number of bytes successfully received in tickISR() (see isr_rx example)
uint8_t getRSSI(); // get reception quality (percentage of successful transmissions 0.. 100)

uint8_tbuffer[]; // access to the receive buffer
uint8_t byteBuf; // access to the received byte buffer

// ============= CRC ==============
// you can use built-in functions to generate a CRC byte for manually packing packets
uint8_t G433_crc8(uint8_t *buffer, uint8_t size); // manual CRC8
uint8_t G433_crc_xor(uint8_t *buffer, uint8_t size); // manual CRC XOR

// ====== DEFINE-SETTINGS ======
// call before linking the library
#define G433_FAST // [TX] fast sync for green modules
#define G433_MEDIUM // [TX] average timing when sent to SYN480R OVER 400ms (active by default)
#define G433_SLOW // [TX] long sync when sent to SYN480R LESS than 400ms
#define G433_SPEED 1000 // [RX/TX] speed, should be the same on RX and TX, 100-10000 bps, default 2000 bps
#define G433_RSSI_COUNT 8 // [RX] number of successfully received packets for RSSI calculation (default 8)
#define G433_CUT_RSSI // [RX] remove RSSI calculation from code (save some memory)
```

### Receiver operation without interruption
To receive data, it is enough to call the `tick()` method in the main loop, it polls the radio pin itself.
- If there are blocking places in the program and loop() is executed with delays - there will be data loss
- For a more reliable reception, it is recommended to call `tickWait()`, it blocks the execution of the code for the duration of the reception.

### Receiver operation with interrupts
It is recommended to use the CHANGE interrupt technique by calling `tickISR()` in it.
Then the reception will be asynchronous and will work even in the loaded program.
- If the standard interrupt pins are not enough, you can use PCINT. Manually or withLibraries https://github.com/NicoHood/PinChangeInterrupt

<a id="example"></a>
## Examples
See **examples** for other examples!
### Sending
```cpp
#include <Gyver433.h>
Gyver433_TX<2>tx; // specified pin

void setup() {
}

chardata[] = "Hello from #xx"; // string to send
byte count = 0; // counter to send

void loop() {
  // add counter to string
  data[12] = (count / 10) + '0';
  data[13] = (count % 10) + '0';
  if (++count >= 100) count = 0;
  tx.sendData(data);
  delay(100);
}
```

### Receive in interrupt
```cpp
#include <Gyver433.h>
Gyver433_RX<2, 20> rx; // specified pin and buffer size

void setup() {
  Serial.begin(9600);
  attachInterrupt(0, isr, CHANGE); // interrupt radio pin by CHANGE
}

// spec. ticker is called in interrupt
void isr() {
  rx.tickISR();
}

void loop() {
  if (rx.gotData()) { // if more than 0 successfully received
    Serial.write(rx.buffer, rx.size); // output
    Serial.println();
  }
  delay(50); // simulate loaded code
}
```

<a id="versions"></a>
## Versions
- v1.0
- v1.1 - optimization, new interface, support for cheap blue modules, work in interrupt
- v1.2 - improving the quality of communication, optimization of work in interruption
- v1.3 - added RSSI output
- v1.4 - redesigned FastIO
- v1.4.1 - FastIO removed, CRC moved separately
- v2.0 - removed buffer for sending, removed Manchester, completely redone and optimized communication interface
- v2.0.1 - fix compiler warnings

<a id="feedback"></a>
## Bugs and feedback
When you find bugs, create an **Issue**, or better, immediately write to the mail [alex@alexgyver.ru](mailto:alex@alexgyver.ru)
The library is open for revision and your **Pull Request**'s!
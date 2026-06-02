This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/Gyver433.svg?color=brightgreen)](https://github.com/GyverLibs/Gyver433/releases/latest/download/Gyver433.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/Gyver433.svg)](https://registry.platformio.org/libraries/gyverlibs/Gyver433)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/Gyver433?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

> [!NOTE]
> **There is an easier and more convenient library.[GyverWire](https://github.com/GyverLibs/GyverWire)**

# Gyver433
Library for 433 MHz and Arduino radio modules
- ** Data transfer from module to module via its own communication interface**, i.e. does not work with Chinese remotes and gates!
- Support for Chinese module curves
- Integrated CRC Integrity Control
- Accelerated IO Algorithm for AVR Arduino
- Asynchronous reception in interruption
- Super light liba, even tinni13

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)
- When connecting an interrupt to esp8266, remember the attribute`IRAM_ATTR`

## Contents
- [Installation](#install)
- [Interface.](#interface)
- [Connection](#wiring)
- [Initialization](#init)
- [Use of use](#usage)
- [Example](#example)
- [Versions](#versions)
- [Bugs and feedback](#feedback)

<a id="install"></a>
## Installation
- The library can be found under the name **Gyver433** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/Gyver433/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="interface"></a>
## Interface.
- Before the data is sent, there is a synchronizing pulse, *TRAIN* ms.
- Next up is momentum, *START*ms, LOW
- Data is encoded by pulse length
- Completes sending stop pulse, *TRAIN*ms.
![scheme](/doc/interface.png)
![scheme](/doc/timing.png)

<a id="wiring"></a>
## Connection
![scheme](/doc/radio433.png)
Transmitters:
- SYN115, small chip: 1.8-3.6V, max speed 8000
- FS1000A: 3-12V, maximum speed of 10,000
- WL102-341: 2.0-3.6V, max speed 6000

Receivers:
- SYN480R, large chip: 3.3-5.5V
- MX-RM-5V (RF-5V): 5V
- RX470 (WL101-341): 3-5V

![scheme](/doc/scheme.jpg)
![scheme](/doc/radio.png)

<a id="init"></a>
## Initialization
```cpp
// === Transmitter ===
Gyver433_TX<пин> tx;
Gyver433_TX<пин, CRC> tx;

// = = = = = TRACK ==
Gyver433_RX<пин> rx;
Gyver433_RX<пин, буфер> rx;
Gyver433_RX<пин, буфер, CRC> rx;

// pin: digital pin
// buffer: buffer size in bytes, silent. 64
// CRC: Data integrity check: G433 CRC8 (reliable), G433 XOR (light), G433 NOCRC (disabled). Shut up. G433 CRC8
```

<a id="usage"></a>
## Use of use
```cpp
// ========= Gyver433_TX =========
void sendData(T &data);                 // Send any type of data (CRC is added automatically)
void write(uint8_t* buf, uint8_t size); // send an array of bytes of specified size (CRC not added)

// ========= Gyver433_RX =========
// interruption
void tickISR();             // Reception ticker to call in CHANGE interrupt
uint8_t tickISRraw();       // Manual reception in CHANGE interruption. Return 1 (beginning of reception), 2 (byte received), 3 (end of packet)

// loop
uint16_t tick();            // non-blocking reception. Return the number of successfully accepted bytes
uint16_t tickWait();        // blocking reception. Return the number of successfully accepted bytes

bool readData(T &data);     // read the buffer in any data type (in the specified variable)
uint16_t getSize();         // obtain the size of the received data
uint16_t gotData();         // return the number of bytes successfully received in tickISR() (see example isr rx)
uint8_t getRSSI();          // Receive quality (percentage of successful transmissions 0) 100)

uint8_t buffer[];           // reception buffer
uint8_t byteBuf;            // byte buffer

// ============= CRC =============
// You can use built-in functions to generate a CRC byte for manual packing
uint8_t G433_crc8(uint8_t *buffer, uint8_t size);       // hand-held CRC8
uint8_t G433_crc_xor(uint8_t *buffer, uint8_t size);    // hand-held CRC XOR

// ====== DEFINIONS-STREAMS ======
// call up
#define G433_FAST           // [TX] Short synchronization for green modules
#define G433_MEDIUM         // [TX] Average synchronization when sent to SYN480R MORE than 400ms (active by default)
#define G433_SLOW           // [TX] long synchronization when sent to SYN480R less than 400ms
#define G433_SPEED 1000     // [RX/TX] speed, should be the same on RX and TX, 100-10000 bits/s, silent. 2000 bits/s
#define G433_RSSI_COUNT 8   // [RX] Number of successfully received RSSI packets (by default). (8)
#define G433_CUT_RSSI       // [RX] Remove RSSI calculation from code (saves a little memory)
```

### Receiver operation without interruption
To receive data, it is enough to call the method in the main cycle`tick()`He's interviewing the pin radio himself.
- If the program has blocking places and loop() is executed with delays, there will be a loss of data.
- For a more reliable reception, it is recommended to call`tickWait()`It blocks the execution of the code during the reception.

### Interrupted receiver operation
It is recommended to use the reception in an interrupt by CHANGE, for this you need to call in it.`tickISR()`. 
Then the reception will be asynchronous and will work even in the downloaded program.
- If standard interrupt pins are not enough, you can use PCINT. manually or through the libraryhttps://github.com/NicoHood/PinChangeInterrupt

<a id="example"></a>
## Examples
For more examples see **examples**!
### Sending.
```cpp
#include <Gyver433.h>
Gyver433_TX<2> tx;  // pinned

void setup() {
}

char data[] = "Hello from #xx"; // line
byte count = 0;                 // dispatcher

void loop() {
  // add the counter to the line
  data[12] = (count / 10) + '0';
  data[13] = (count % 10) + '0';
  if (++count >= 100) count = 0;
  tx.sendData(data);
  delay(100);
}
```

### Interrupted reception
```cpp
#include <Gyver433.h>
Gyver433_RX<2, 20> rx;  // specified pin and buffer size

void setup() {
  Serial.begin(9600);  
  attachInterrupt(0, isr, CHANGE);  // CHANGE
}

// Special ticker called in interruption
void isr() {
  rx.tickISR();
}

void loop() {
  if (rx.gotData()) {                   // If more than 0 is successfully accepted
    Serial.write(rx.buffer, rx.size);   // output
    Serial.println();
  }  
  delay(50);                            // imitation
}
```

<a id="versions"></a>
## Versions
- v1.0
- v1.1 - optimization, new interface, support for cheap blue modules, work in interruption
- v1.2 - improving communication quality, optimizing work in interruption
- v1.3 - RSSI output added
- v1.4 - redesigned FastIO
- v1.4.1 - Removed by FastIO, CRC issued separately
- v2.0 - Removed buffer to send, removed the Manchester, completely redesigned and optimized communication interface
- v2.0.1 - fix compiler warnings

<a id="feedback"></a>
## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.

This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gyver433
Library for Radio Modules 433 MHz and Arduino
- ** Data transfer from the module to the module on your own communication interface **, i.e.Does not work with Chinese remote controls and barriers!
- Support for crooked Chinese modules
- Built -in CRC Control of integrity
- Accelerated IO algorithm for AVR Arduino
- asynchronous reception in interruption
- Super lightly Liba, will even start on Tini13

## compatibility
Compatible with all arduino platforms (used arduino functions)
- When connecting an interruption on the ESP8266, do not forget the Attricbut `IRAM_ATTR`

## Content
- [installation] (# Install)
- [interface] (#interface)
- [connection] (#wiring)
- [initialization] (#init)
- [use] (#usage)
- [Example] (# Example)
- [versions] (#varsions)
- [bugs and feedback] (#fedback)

<a id="install"> </a>
## Installation
- The library can be found by the name ** Gyver433 ** and installed through the library manager in:
    - Arduino ide
    - Arduino ide v2
    - Platformio
- [download the library] (https://github.com/gyverlibs/gyver433/archive/refs/heads/main.zip) .Zip archive for manual installation:
    - unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
    - unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
    - unpack and put in *documents/arduino/libraries/ *
    - (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%BD%D0%BE%BE%BE%BED0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!


<a Id="INterFace"> </a>
## Interface
- Before sending the data, there is a synchronizing pulse, * train * ms
- Next, start impulse, * Start * ms, Low
- Data is encoded by pulse length
- completes the sending of the stop impulse, * train * ms
! [Scheme] (/doc/interface.png)
! [Scheme] (/doc/timing.png)

<a id="wiring"> </a>
## connection
! [Scheme] (/doc/radio433.png)
Transmitters:
- SYN115, small chip: 1.8-3.6V, max.speed 8000
- FS1000A: 3-12V, Max.speed of 10,000
-WL102-341: 2.0-3.6V, max.speed 6000

Receivers:
- SYN480R, large chip: 3.3-5.5V
-MX-RM-5V (RF-5V): 5V
-RX470 (WL101-341): 3-5V

! [Scheme] (/doc/scheme.jpg)
! [Scheme] (/doc/radio.png)

<a id="init"> </a>
## initialization
`` `CPP
// === transmitter ===
Gyver433_TX <PIN> TX;
Gyver433_TX <PIN, CRC> TX;

// === The receiver ====
Gyver433_rx <PIN> RX;
Gyver433_rx <pin, buffer> rx;
Gyver433_rx <pin, buffer, CRC> rx;

// PIN: Digital PIN
// buffer: the size of the buffer in bytes, by the silence.64
// CRC: Statement of data integrity: G433_CRC8 (reliable), G433_XOR (Light), G433_NOCRC (disconnected).By the silence.G433_CRC8
`` `

<a id="usage"> </a>
## Usage`` `CPP
// ========= gyver433_TX ============
VOID SendData (T & Data);// Send data any type (CRC is added automatically)
VOID Write (uint8_t* buf, uint8_t size);// Send an array of byte of the specified size (CRC is not added)

// ========= gyver433_rx ============
// Reception in interruption
VOID Tickisr ();// Take of reception for calling in interruption by Change
uint8_t TickisrRaw ();// manual reception in the interruption in Change.Return 1 (beginning of reception), 2 (Baite adopted), 3 (end of the package)

// Reception in Loop
uint16_t tick ();// Neboking technique.Will return the number of successfully accepted bytes
uint16_t tickwait ();// blocking reception.Will return the number of successfully accepted bytes

Bool Readdata (T & Data);// read the buffer in any type of data (in the specified variable)
uint16_t getsize ();// Get the amount of data accepted
uint16_t gotdata ();// will return the number of byte successfully accepted in Tickisr () (see ISR_RX example)
uint8_t getrssi ();// get the quality of the reception (percentage of successful gears 0 .. 100)

uint8_t buffer [];// Access to the reception buffer
uint8_t bytebuf;// Access to the Baita Boofer

// ============== CRC ==================
// you can use the built -in functions for generating the CRC byt for manual packages
uint8_t g433_crc8 (uint8_t *buffer, uint8_t size);// manual CRC8
uint8_t g433_crc_xor (uint8_t *buffer, uint8_t size);// manual CRC XOR

// ====== Defain-settings ==========
// Call before connecting the library
#define g433_fast // [tx] short synchronization for green modules
#define g433_Medium // [tx] average synchronization when sending to SYN480R more than 400MS (actively by default)
#define g433_slow // [tx] Long synchronization when sending to SYN480R less often 400ms
#define G433_Speed 1000 // [RX/TX] Speed, should be the same on RX and TX, 100-10,000 bits/s, silent.2000 bits/s
#define g433_rssi_count 8 // [RX] The number of successfully accepted packets for RSSI calculation (according to default. 8)
#define g433_cut_rssi // [rx] Remove the RSSI calculation from the code (save a little memory)
`` `

### Work of the receiver without interruption
To receive data, it is enough to cause a basic cycle of the `tick ()` method, he himself interviews a pin of the radio.
- If the program has blocking places and Loop () is performed with delays - there will be a loss of data
- For more reliable reception, it is recommended to call `tickwait ()`, it blocks the performance of the code during the reception.

### Work of the receiver with interruptions
It is recommended to use the technique in the interrupt on Change, for this you need to call `Tickisr ()` in it.
Then the reception will be asynchronous and will work even in a loaded program.
- If the standard pins of interruptions are not enough, you can use PCINT.Manually or with the help of the library https://github.com/nicohood/pinchangeinterrapt

<a id="EXAMPLE"> </a>
## Examples
The rest of the examples look at ** Examples **!
### Sending
`` `CPP
#include <gyver433.h>
Gyver433_tx <2> tx;// indicated the pin

VOID setup () {
}

Char Data [] = "Hello from #xx";// line for sending
Byte count = 0;// counter for sending

VOID loop () {
  // Add the counter to the line
  Data [12] = (Count / 10) + '0';
  Data [13] = (Count % 10) + '0';
  if (++ count> = 100) count = 0;
  TX.SendData (Data);
  DELAY (100);
}
`` `

### Reception in interruption
`` `CPP
#include <gyver433.h>
Gyver433_rx <2, 20> rx;// indicated the pin and size of the buffer

VOID setup () {
  Serial.Begin (9600);
  Attachinterrupt (0, Isr, Change);// Interruption of Pina Radio on Change
}

// Special.The ticer is called in interruption
VOID isr () {
  rx.tickisr ();
}

VOID loop () {
  if (rx.gotdata ()) {// If more than 0 successfully accepted
    Serial.write (rx.buffer, rx.size);// Display
    Serial.println ();
  }
  DELAY (50);// imitation of a loaded code
}
`` `

<a id="versions"> </a>
## versions
- V1.0
- V1.1 - optimization, new interface, support for cheap blue modules, work in interruption
- V1.2 -improvement in communication quality, optimization of work in interruption
- v1.3 - added RSSI withdrawal
- V1.4 - Redeled Fastio
- v1.4.1 - removed Fastio, CRC is taken separately
- V2.0 - removed the buffer for sending, removed Manchester, completely redone and optimized the communication interface
- V2.0.1 - FIX Compiler Warnings

<a id="feedback"> </a>
## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!


When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code
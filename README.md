![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)
![author](https://img.shields.io/badge/author-AlexGyver-informational.svg)
# Gyver433
Библиотека для радиомодулей 433 МГц и Arduino
- Супер лёгкая либа, заведётся даже на тини13 (отправка)
- Поддержка кривых китайских модулей
- Интерфейс Manchester Coding (v1.1)
- Встроенный CRC контроль целостности (CRC8 или XOR)
- Ускоренный алгоритм IO для AVR Arduino
- Опционально работа в прерывании (приём данных)

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Установка](#install)
- [Инициализация](#init)
- [Использование](#usage)
- [Пример](#example)
- [Версии](#versions)
- [Баги и обратная связь](#feedback)

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **Gyver433** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/Gyver433/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

<a id="init"></a>
## Инициализация
```cpp
//Классы:
Gyver433_RX // приёмник
Gyver433_TX // передатчик

Gyver433_xx<пин, буфер, CRC> xx;
// пин: цифровой пин
// буфер: размер буфера в байтах. На "ручную" отправку буфер не нужен (пример raw_tx). По умолч. 64 байта
// CRC: проверка целостности данных: G433_CRC8 (надёжный), G433_XOR (лёгкий), G433_NOCRC (отключено). По умолч. G433_CRC8

// Дефайны-настройки перед подключением библиотеки
#define G433_SLOW_MODE      // "медленный режим" для синих модулей SYN480R
#define G433_SPEED 1000     // скорость 100-8000 бит/с, по умолч. 2000 бит/с
```

<a id="usage"></a>
## Использование
```cpp
// ========= Gyver433_TX =========
void sendData(T &data);                 // отправить данные любого типа (CRC добавится автоматически)
void write(uint8_t* buf, uint8_t size); // отправить массив байт указанного размера (CRC не добавляется)
uint8_t buffer[];                       // доступ к буферу для отладки

// ========= Gyver433_RX =========
uint8_t tick();             // неблокирующий приём, вернёт кол-во успешно принятых байт
uint8_t tickWait();         // блокирующий приём, вернёт кол-во успешно принятых байт
bool readData(T &data);     // прочитает буфер в любой тип данных (в указанную переменную)
int getSize();              // получить размер принятых данных
bool gotData();             // вернёт true при получении корректных данных (если tick опрашивается в другом месте)
uint8_t buffer[];           // доступ к буферу для отладки

// ============= CRC =============
// можно использовать встроенные функции для генерации байта CRC для ручной упаковки пакетов
uint8_t G433_crc8(uint8_t *buffer, uint8_t size);       // ручной CRC8
uint8_t G433_crc_xor(uint8_t *buffer, uint8_t size);    // ручной CRC XOR
```

<a id="example"></a>
## Пример
Остальные примеры смотри в **examples**!  
### Отправка
```cpp
// ======== ПЕРЕДАТЧИК =========
#define G433_SLOW_MODE
#include <Gyver433.h>
Gyver433_TX<2, 20> tx;  // указали пин и размер буфера

void setup() {
}

char data[] = "Hello from #xx"; // строка для отправки
byte count = 0;                 // счётчик для отправки

void loop() {
  // добавляем счётчик в строку
  data[12] = (count / 10) + '0';
  data[13] = (count % 10) + '0';
  if (++count >= 100) count = 0;
  tx.sendData(data);
  delay(100);
}

// ======== ПРИЁМНИК =========
#define G433_SLOW_MODE
#include <Gyver433.h>
Gyver433_RX<2, 20> rx;  // указали пин и размер буфера

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (rx.tickWait()) {
    Serial.write(rx.buffer, rx.size);
    Serial.println();
  }
}
```

### Приём
```cpp
// крупный приёмник 5.0 SYN480R

#define G433_BUFSIZE 50   // размер буфера
#define G433_SPEED 2000   // скорость бит/сек (минимальная)

#include <Gyver433.h>
Gyver433_RX rx(2);

void setup() {
  Serial.begin(9600);
}


void loop() {
  // tick принимает асинхронно, но может ловить ошибки при загруженном коде
  // tickWait блокирует выполнение, но принимает данные чётко
  if (rx.tickWait()) {
    byte buf[64];
    rx.readData(buf);
    for (byte i = 0; i < rx.size; i++) Serial.write(buf[i]);
  }
}
```

<a id="versions"></a>
## Версии
- v1.0
- v1.1 - оптимизация, новый интерфейс, поддержка дешёвых синих модулей, работа в прерывании

<a id="feedback"></a>
## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!
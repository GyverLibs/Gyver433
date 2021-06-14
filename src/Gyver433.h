/*
    Библиотека для радиомодулей 433 МГц и Arduino
    Документация: 
    GitHub: https://github.com/GyverLibs/Gyver433
    Возможности:
    - Не использует прерывания и таймеры (кроме нулевого, читает micros())
    - Встроенный CRC контроль целостности
    - Ускоренный алгоритм IO для AVR Arduino
    - Работает с хорошими и плохими 433 МГц модулями
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0 - релиз
    v1.1 - оптимизация, новый интерфейс, поддержка дешёвых синих модулей, работа в прерывании
*/

#ifndef Gyver433_h
#define Gyver433_h
#include <Arduino.h>
#include "FastIO.h"

// настройки из скетча:
// #define G433_SLOW_MODE - включить "медленный режим" для плохих модулей
// #define G433_SPEED n - скорость, бит/сек. Рекомендуется 2000. Работает вплоть до 6000

uint8_t G433_crc8(uint8_t *buffer, uint8_t size);       // ручной CRC8
uint8_t G433_crc_xor(uint8_t *buffer, uint8_t size);    // ручной CRC XOR
#define TRAINING_TIME_SLOW (500000ul)                   // время синхронизации для SLOW_MODE

// =========================================================================
#ifndef G433_SPEED
#define G433_SPEED 3000     // скорость по умолчанию
#endif

// тайминги интерфейса
#define FRAME_TIME (1000000ul / G433_SPEED)     // время фрейма
#define HALF_FRAME (FRAME_TIME / 2)             // полфрейма
#define START_PULSE (FRAME_TIME * 2)            // стартовый импульс
#define TRAINING_AMOUNT_SLOW (TRAINING_TIME_SLOW / FRAME_TIME / 2)  // количество импульсов для SLOW_MODE

// количество импульсов в зависимости от SLOW_MODE
#ifdef G433_SLOW_MODE
#define TRAINING_AMOUNT 40
#else
#define TRAINING_AMOUNT 10
#endif

// окно времени для обработки старта и фрейма
#define START_MIN (START_PULSE * 3 / 4)
#define START_MAX (START_PULSE * 5 / 4)
#define FRAME_MIN (FRAME_TIME * 3 / 4)
#define FRAME_MAX (FRAME_TIME * 5 / 4)

// жоский delay для avr
#ifdef AVR
#define G433_DELAY(x) _delay_us(x)
#else
#define G433_DELAY(x) delayMicroseconds(x)
#endif

// режимы CRC
#define G433_CRC8 0
#define G433_XOR 1
#define G433_NOCRC 2

// crc8 один байт
void G433_crc8_byte(uint8_t &crc, uint8_t data);

// ============ ПЕРЕДАТЧИК ============
template <uint8_t TX_PIN, uint16_t TX_BUF = 64, uint8_t CRC_MODE = G433_CRC8>
class Gyver433_TX {
public:
    Gyver433_TX() {
        pinMode(TX_PIN, OUTPUT);
    }
    
    // отправка, блокирующая. Кушает любой тип данных
    template <typename T>
    void sendData(T &data) {
        const uint8_t *ptr = (const uint8_t*) &data;
        for (uint16_t i = 0; i < sizeof(T); i++) buffer[i] = *ptr++;
        if (CRC_MODE == G433_CRC8) {
            buffer[sizeof(T)] = G433_crc8(buffer, sizeof(T));
            write(buffer, sizeof(T) + 1);
        } else if (CRC_MODE == G433_XOR) {
            buffer[sizeof(T)] = G433_crc_xor(buffer, sizeof(T));
            write(buffer, sizeof(T) + 1);
        } else {
            write(buffer, sizeof(T));
        }
    }
    
    // отправка сырого набора байтов
    void write(uint8_t* buf, uint16_t size) {
        #ifdef G433_SLOW_MODE
        for (uint16_t i = 0; i < ((millis() - tmr > 400) ? TRAINING_AMOUNT_SLOW : TRAINING_AMOUNT); i++) {
        #else
        for (uint16_t i = 0; i < TRAINING_AMOUNT; i++) {
        #endif        
            fastWrite(TX_PIN, 1);
            G433_DELAY(FRAME_TIME);
            fastWrite(TX_PIN, 0);
            G433_DELAY(FRAME_TIME);
        }
        fastWrite(TX_PIN, 1);         // старт
        G433_DELAY(START_PULSE);      // ждём
        fastWrite(TX_PIN, 0);         // старт бит
        G433_DELAY(HALF_FRAME);       // ждём
        for (uint16_t n = 0; n < size; n++) {
            uint8_t data = buf[n];
            for (uint8_t b = 0; b < 8; b++) {
                fastWrite(TX_PIN, !(data & 1));
                G433_DELAY(HALF_FRAME);
                fastWrite(TX_PIN, (data & 1));                
                G433_DELAY(HALF_FRAME);
                data >>= 1;
            }
        }
        fastWrite(TX_PIN, 0);   // конец передачи
        #ifdef G433_SLOW_MODE
        tmr = millis();
        #endif
        }
        
        // доступ к буферу
        uint8_t buffer[TX_BUF];
        
    private:    
        #ifdef G433_SLOW_MODE
        uint32_t tmr = 0;
        #endif
    };

    // ============ ПРИЁМНИК ============
    template <uint8_t RX_PIN, uint16_t RX_BUF = 64, uint8_t CRC_MODE = G433_CRC8>
    class Gyver433_RX {
    public:
        Gyver433_RX() {
            pinMode(RX_PIN, INPUT);
        }
        
        // неблокирующий приём, вернёт кол-во успешно принятых байт
        uint8_t tick() {
            uint32_t thisPulse = micros() - tmr;                // время импульса
            if (parse == 2 && thisPulse >= FRAME_TIME * 2) {    // фрейм не закрыт
                parse = size = 0;                               // приём окончен   
                if (byteCount > 1) {                            // если что то приняли
                    if (CRC_MODE == G433_CRC8) {                // CRC8 
                        if (!G433_crc8(buffer, byteCount)) {
                            size = byteCount - 2;
                            dataReady = 1;
                        }
                    } else if (CRC_MODE == G433_XOR) {          // CRC XOR
                        if (!G433_crc_xor(buffer, byteCount)) {
                            size = byteCount - 2;
                            dataReady = 1;
                        }
                    } else {                                    // без CRC
                        size = byteCount - 1;
                        dataReady = 1;
                    }
                }
                return size;
            }
            bool bit = fastRead(RX_PIN);                        // читаем пин
            if (bit != prevBit) {  		                        // ловим изменение сигнала
                if (parse == 1) {   			                // в прошлый раз поймали фронт
                    tmr += thisPulse;                           // сброс таймера
                    if (thisPulse > START_MIN && thisPulse < START_MAX) {   // старт бит?
                        parse = 2;                                          // ключ на старт
                        byteCount = bitCount = size = 0;                    // сброс
                        dataReady = 0;
                        for (uint8_t i = 0; i < RX_BUF; i++) buffer[i] = 0; // чистим буфер
                    } else parse = 0;									    // не старт бит
                } else if (parse == 2) {		                            // идёт парсинг                
                    if (thisPulse > FRAME_MIN && thisPulse < FRAME_MAX) {   // фронт внутри таймфрейма
                        tmr += thisPulse;                                   // синхронизируем тайминги
                        buffer[byteCount] >>= 1;                            // двигаем байт
                        if (bit && !prevBit) buffer[byteCount] |= _BV(7);   // пишем единичку
                        bitCount++;                                         // счётчик битов
                    }
                    if (bitCount == 8) {                        // собрали байт
                        bitCount = 0;                           // сброс
                        if (++byteCount >= RX_BUF) parse = 0;   // буфер переполнен
                    }
                }

                if (bit && !prevBit && parse == 0) {            // ловим фронт
                    parse = 1;									// флаг на старт
                    tmr += thisPulse;                           // сброс таймера
                }
                prevBit = bit;
            }
            return 0;
        }
        
        // блокирующий приём, вернёт кол-во успешно принятых байт
        uint8_t tickWait() {
            do {
                if (tick()) return size;
            } while (parse == 2);
            return 0;
        }
        
        // прочитает буфер в любой тип данных
        template <typename T>
        bool readData(T &data) {
            if (sizeof(T) > RX_BUF) return false;		
            uint8_t *ptr = (uint8_t*) &data;	
            for (uint16_t i = 0; i < sizeof(T); i++) *ptr++ = buffer[i];
            return true;
        }
        
        // вернёт true при получении корректных данных
        bool gotData() {
            tick();
            if (dataReady) {
                dataReady = 0;
                return 1;
            } return 0;
        }
        
        // получить размер принятых данных
        int getSize() {
            return size;
        }
        
        // размер принятых данных
        int size = 0;
        
        // доступ к буферу
        uint8_t buffer[RX_BUF];
        
    private:    
        bool prevBit, dataReady = 0;
        uint8_t parse = 0;
        uint32_t tmr = 0;
        uint8_t bitCount = 0, byteCount = 0;
    };

    // ===== CRC =====
    void G433_crc8_byte(uint8_t &crc, uint8_t data) {
#if defined (__AVR__)
        // резкий алгоритм для AVR
        uint8_t counter;
        uint8_t buffer;
        asm volatile (
        "EOR %[crc_out], %[data_in] \n\t"
        "LDI %[counter], 8          \n\t"
        "LDI %[buffer], 0x8C        \n\t"
        "_loop_start_%=:            \n\t"
        "LSR %[crc_out]             \n\t"
        "BRCC _loop_end_%=          \n\t"
        "EOR %[crc_out], %[buffer]  \n\t"
        "_loop_end_%=:              \n\t"
        "DEC %[counter]             \n\t"
        "BRNE _loop_start_%="
        : [crc_out]"=r" (crc), [counter]"=d" (counter), [buffer]"=d" (buffer)
        : [crc_in]"0" (crc), [data_in]"r" (data)
        );
#else
        // обычный для всех остальных
        uint8_t i = 8;
        while (i--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
#endif
    }

    uint8_t G433_crc8(uint8_t *buffer, uint8_t size) {
        uint8_t crc = 0;
        for (uint8_t i = 0; i < size; i++) G433_crc8_byte(crc, buffer[i]);
        return crc;
    }
    uint8_t G433_crc_xor(uint8_t *buffer, uint8_t size) {
        uint8_t crc = 0;
        for (uint8_t i = 0; i < size; i++) crc ^= buffer[i];
        return crc;
    }
#endif
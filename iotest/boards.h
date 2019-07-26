#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)


// Thank you Elmue !
#if defined(TEENSYDUINO) 

    //  --------------- Teensy -----------------

    #if defined(__AVR_ATmega32U4__)
        #define BOARD "Teensy 2.0"
    #elif defined(__AVR_AT90USB1286__)       
        #define BOARD "Teensy++ 2.0"
    #elif defined(__MK20DX128__)       
        #define BOARD "Teensy 3.0"
    #elif defined(__MK20DX256__)       
        #define BOARD "Teensy 3.2" // and Teensy 3.1 (obsolete)
    #elif defined(__MKL26Z64__)       
        #define BOARD "Teensy LC"
    #elif defined(__MK64FX512__)
        #define BOARD "Teensy 3.5"
    #elif defined(__MK66FX1M0__)
        #define BOARD "Teensy 3.6"
    #else
       #error "Unknown board"
    #endif

#else // --------------- Arduino ------------------

    #if   defined(ARDUINO_AVR_ADK)       
        #define BOARD "Mega Adk"
    #elif defined(ARDUINO_AVR_BT)    // Bluetooth
        #define BOARD "Bt"
    #elif defined(ARDUINO_AVR_DUEMILANOVE)       
        #define BOARD "Duemilanove"
    #elif defined(ARDUINO_AVR_ESPLORA)       
        #define BOARD "Esplora"
    #elif defined(ARDUINO_AVR_ETHERNET)       
        #define BOARD "Ethernet"
    #elif defined(ARDUINO_AVR_FIO)       
        #define BOARD "Fio"
    #elif defined(ARDUINO_AVR_GEMMA)
        #define BOARD "Gemma"
    #elif defined(ARDUINO_AVR_LEONARDO)       
        #define BOARD "Leonardo"
        #define DIGITAL_PIN_COUNT 14
        #define ANALOG_PIN_COUNT 6
        HardwareSerial& chainSerial = Serial1;
        Serial_& pcSerial = Serial;
        
        #ifdef MASTER
        Serial_& dataSerial = Serial;
        #endif
        #ifdef SLAVE
        HardwareSerial& dataSerial = Serial1;
        #endif
    #elif defined(ARDUINO_AVR_LILYPAD)
        #define BOARD "Lilypad"
    #elif defined(ARDUINO_AVR_LILYPAD_USB)
        #define BOARD "Lilypad Usb"
    #elif defined(ARDUINO_AVR_MEGA)       
        #define BOARD "Mega"
    #elif defined(ARDUINO_AVR_MEGA2560)       
        #define BOARD "Mega 2560"
        #define DIGITAL_PIN_COUNT 54
        #define ANALOG_PIN_COUNT 16
        HardwareSerial& chainSerial = Serial1;
        HardwareSerial& pcSerial = Serial;
      
        #ifdef MASTER
        HardwareSerial& dataSerial = Serial;
        #endif
        #ifdef SLAVE
        HardwareSerial& dataSerial = Serial1;
        #endif
        
    #elif defined(ARDUINO_AVR_MICRO)       
        #define BOARD "Micro"
        #define DIGITAL_PIN_COUNT 14
        #define ANALOG_PIN_COUNT 6
        HardwareSerial& chainSerial = Serial1;
        Serial_& pcSerial = Serial;
        
        #ifdef MASTER
        Serial_& dataSerial = Serial;
        #endif
        #ifdef SLAVE
        HardwareSerial& dataSerial = Serial1;
        #endif
    #elif defined(ARDUINO_AVR_MINI)       
        #define BOARD "Mini"
    #elif defined(ARDUINO_AVR_NANO)       
        #define BOARD "Nano"
        #define DIGITAL_PIN_COUNT 14
        #define ANALOG_PIN_COUNT 8
        HardwareSerial& chainSerial = Serial;
        HardwareSerial& pcSerial = Serial;
        
        #ifdef MASTER
        HardwareSerial& dataSerial = Serial;
        #endif
        #ifdef SLAVE
        HardwareSerial& dataSerial = Serial;
        #endif
    #elif defined(ARDUINO_AVR_NG)       
        #define BOARD "NG"
    #elif defined(ARDUINO_AVR_PRO)       
        #define BOARD "Pro"
    #elif defined(ARDUINO_AVR_ROBOT_CONTROL)       
        #define BOARD "Robot Ctrl"
    #elif defined(ARDUINO_AVR_ROBOT_MOTOR)       
        #define BOARD "Robot Motor"
    #elif defined(ARDUINO_AVR_UNO)       
        #define BOARD "Uno"
        #define DIGITAL_PIN_COUNT 14
        #define ANALOG_PIN_COUNT 6
        HardwareSerial& chainSerial = Serial;
        HardwareSerial& pcSerial = Serial;
        
        #ifdef MASTER
        HardwareSerial& dataSerial = Serial;
        #endif
        #ifdef SLAVE
        HardwareSerial& dataSerial = Serial;
        #endif
    #elif defined(ARDUINO_AVR_YUN)       
        #define BOARD "Yun"

    // These boards must be installed separately:
    #elif defined(ARDUINO_SAM_DUE)       
        #define BOARD "Due"
    #elif defined(ARDUINO_SAMD_ZERO)       
        #define BOARD "Zero"
    #elif defined(ARDUINO_ARC32_TOOLS)       
        #define BOARD "101"
    #else
       #error "Unknown board"
       #pragma message(VAR_NAME_VALUE(BOARD))
    #endif

#endif

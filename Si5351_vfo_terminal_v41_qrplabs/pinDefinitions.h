#include <arduino.h>
// ++++++++++++++++++++++++++++++++++++++++++
//fuer andere Hardware PIN Definitionen
// anpassen !!
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
#define SCL A5
#define SDA A4
#define RX  0
#define TX  1
#endif

#ifdef ARDUINO_attiny
#define SCL 2
#define SDA 0
#define RX  3
#define TX  4
#endif

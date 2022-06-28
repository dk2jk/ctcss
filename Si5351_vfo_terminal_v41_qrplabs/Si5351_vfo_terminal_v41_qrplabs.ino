#include "pinDefinitions.h"

//libraries
#include <SoftwareSerial.h>
#include "si5351_lite.h"
#include "SoftI2CMaster.h"
#include <EEPROM.h>

//übersicht
//Class              Object           //Methoden
SoftwareSerial       mySerial(RX, TX);//print, read, available
si5351_lite          si5351;          //setfreq,setDrive
//SoftI2CMaster      myI2C(SCL,SDA);  //start,read,write,stop( wird von si5351 verwendet )
//static EEPROMClass EEPROM;          //put, get ( ClassName EEPROM vordefiniert)

/***
  "quarzersatz"
   Version: ..\Si5351_vfo_terminal_xxx.ino
   vom Aug 24 2018
   NNNNNNNNa : Frequenzeingabe NNNNNNNN in Hertz
          xd : Drive ( 0= 2mA, 1= 4mA, 2= 6ma, 3= 8mA)
           ? : Display Version und aktuelle Daten

   Si-Modul Quarz Einstellen siehe "si5351_lite.h"
   #define XTAL 27005000
   si5351 lite version ist wesentlich kuerzer als die
   offizielle si5351 version von adafruit

   Software Serial benutzt beliebige pins
   fuer rx und tx und ist kürzer als hardware-"Serial"
*/

#define FREQUENZ 63.950
#define STARTFREQUENZ FREQUENZ*1e6
#define STARTDrive 0 /* 2mA */
unsigned long serial_input_number;
const bool OK = true;

const int           EEPROM_STARTADRESSE = 0;
const unsigned long MAGIC_CODE          = 0x12345678L; // magic code
struct eeprom_datensatz {
  unsigned long frequenz;
  unsigned char drive;
  unsigned long identifier;
};
eeprom_datensatz eeData;

////////////////////////////////////////////////////////////////
void setup()
{ mySerial.begin(9600); 
  si5351.init();
  EEPROM.get(EEPROM_STARTADRESSE, eeData);
  if ( eeData.identifier == MAGIC_CODE )  { //frequenz aus eeprom ok -> einstellen
  }
  else  {
    eeData.frequenz     = STARTFREQUENZ;
    eeData.drive        = STARTDrive;
    eeData.identifier   = MAGIC_CODE;
    EEPROM.put(EEPROM_STARTADRESSE, eeData);
  }
  setNewData(&eeData);
  displayHilfe();
}

////////////////////////////////////////////////////////////////
void loop()
{ eingabeInterpreter();
}

////////////////////////////////////////////////////////////////
// interpretiert zeichen von der seriellen schnittstelle
// a) input : ziffern fuer die frequenz in hertz mit abschliessendem 'a'
//    NNNNNNNNa : Frequenzeingabe NNNNNNNN in Hertz
// b) '?'  // display aktuelle frequenz
// c)    xd : Drive x( 0= 2mA, 1= 4mA, 2= 6ma, 3= 8mA")
void  eingabeInterpreter() {
  char c;
  //Check for character
  if (mySerial.available() > 0) {
    c = mySerial.read();
    switch (c) {
      case 'd':
      case 'D':
        if (  serial_input_number > 3 )        {
          error();
        }
        else {
          eeData.drive = serial_input_number;
          update();
        }
        break;
      case 'a': //option a
      case 'A':
        if ( serial_input_number > 0xfffffffeL ) {
          error();
        }
        else {
          eeData.frequenz = serial_input_number;
          update();
        }
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':  // eingabezahl ansammeln
        serial_input_number = serial_input_number * 10 + (c - '0');
        break;
      case '?':
        displayHilfe();
        break;
    }
    mySerial.flush();
  }
}

////////////////////////////////////////////////////////////////
static void displayHilfe()
{ serial_input_number = 0;
  mySerial.println(F(""));
  displayVersion();
  mySerial.print  (F("aktuelle Daten:\n"));
  display();

  mySerial.println (F("NNNNNNNNa : Frequenzeingabe NNNNNNNN in Hertz   "));
  mySerial.println (F("       xd : Drive x( 0= 2mA, 1= 4mA, 2= 6ma, 3= 8mA"));
  mySerial.println (F("        ? : Display Version und aktuelle Daten"));

}

////////////////////////////////////////////////////////////////
static void displayVersion()
{ mySerial.print(F("Version: .."));
  char *s;
  s = strrchr(__FILE__, '\\');
  mySerial.println(s);
  mySerial.println(F("vom " __DATE__ " " __TIME__));
}

////////////////////////////////////////////////////////////////
static void display()
{ mySerial.print  (F( "Frequenz  = "));
  mySerial.print  ( eeData.frequenz );
  mySerial.println(F( " Hertz"));
  mySerial.print  (F( "   Drive  = "));
  mySerial.print  ( ( eeData.drive + 1) * 2 );
  mySerial.println(F( " mA"));
}


////////////////////////////////////////////////////////////////
void setNewData(eeprom_datensatz *ee)
{ si5351.setDrive( 0, ee->drive);
  si5351.setfreq ( 0, ee->frequenz);
}

void update()
{ serial_input_number = 0;
  setNewData(&eeData);
  EEPROM.put(EEPROM_STARTADRESSE, eeData);//write eeprom
  display();
}
void error()
{ serial_input_number = 0;
  mySerial.print(F("wrong number, try again "));
  displayHilfe();
}


#include <Wire.h>
#include "Adafruit_Thermal.h"
//#include "Space.hpp"
//#include "adalogo.h"
//#include "adaqrcode.h"
#include "SoftwareSerial.h"
#define TX_PIN 6
#define RX_PIN 5

bool printReceipt = 0;
byte minute, hour, day, month, year;

SoftwareSerial mySerial(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&mySerial);
int16_t bedrag;
void setup() {
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  pinMode(7, OUTPUT); digitalWrite(7, LOW);

  mySerial.begin(9600);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)

}
void loop() {
  if (printReceipt == 1) {
    printBon();
    printReceipt = 0;
  }
}

void receiveEvent() {
  while (0 < Wire.available()) {
    byte highbyte = Wire.read();
    byte lowbyte = Wire.read();
    minute = Wire.read(); 
    hour = Wire.read();
    day = Wire.read();
    month = Wire.read();
    year= Wire.read();
//    printer.print(hour);
    bedrag = (highbyte << 8) | lowbyte;
  }
  printReceipt = 1;
}
void printBon() {
  printer.setSize('L');
  printer.println(F("OME-DUO"));

  printer.setSize('S');
  printer.inverseOn();
  printer.println(F(".                              ."));
  printer.inverseOff();

  printer.boldOn();
  printer.println(F("Adres:"));
  printer.boldOff();

  printer.justify('L');
  printer.println(F("3025RD\nwijnhaven 107"));
  printer.feed(1);

  printer.boldOn();
  printer.justify('R');
  printer.println(F("Tijd:"));
  printer.boldOff();
  printer.print(hour);
  printer.print(F(":"));
  printer.println(minute);
  printer.justify('L');

  //  addSpaces(lStr , rStr);
  printer.boldOn();
  printer.println(F("Datum:"));
  printer.boldOff();
  printer.print(day);
  printer.print(F("-"));
  printer.print(month);
  printer.print(F("-"));
  printer.println(year);
//  printer.println(F("DD-MM-JJ"))
  printer.feed(1);

  printer.boldOn();
  printer.println(F("ATM Serie#"));

  printer.boldOff();
  printer.println(F("A00A001"));
  printer.feed(1);

  printer.boldOn();
  printer.println(F("IBAN:"));
  printer.boldOff();
  printer.println(F("################0000"));

  printer.boldOn();
  printer.justify('R');
  printer.println(F("Pas Nummer:"));
  printer.boldOff();
  printer.println(F("0000"));
  printer.feed(2);

  printer.justify('C');
  printer.setSize('M');
  printer.println(F("Opgenomen Bedrag"));
  printer.setSize('S');
  printer.print(F("Ð "));
  printer.println(bedrag);
  printer.feed(2);

  printer.boldOn();
  printer.println(F("TOT ZIENS !"));
  printer.boldOff();
  printer.feed(2);

  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

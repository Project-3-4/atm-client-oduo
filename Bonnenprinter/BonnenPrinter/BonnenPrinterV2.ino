
#include <Wire.h>
#include "Adafruit_Thermal.h"
//#include "Space.hpp"
//#include "adalogo.h"
//#include "adaqrcode.h"
#include "SoftwareSerial.h"
#define TX_PIN 6
#define RX_PIN 5
#define motor1a 9
#define motor1b 10
#define motor2a 11
#define motor2b 12

int lastNumbers[6];
long int lastnum;
int passNr;
bool printReceipt = 0;
bool money = false;
byte minute, hour, day, month, year;

SoftwareSerial mySerial(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&mySerial);
int16_t bedrag;
void setup() {
  Serial.begin(9600);
  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);
  Wire.begin(4);
  Wire.onReceive(receiveEvent);
  pinMode(7, OUTPUT); digitalWrite(7, LOW);

  mySerial.begin(9600);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)
}
void loop() {
  delay(5000);
  if (printReceipt == true) {
    array2Lastnum();
    printBon();
    printReceipt = false;
  }
  if (money == true) {
    dispense();
    money = false;
  }
}

void receiveEvent() {
  while (0 < Wire.available()) {
    printReceipt = Wire.read();
    byte highbyte = Wire.read();
    byte lowbyte = Wire.read();
    minute = Wire.read();
    hour = Wire.read();
    day = Wire.read();
    month = Wire.read();
    year = Wire.read();
    for (int i = 0; i < 6; i++) {
      lastNumbers[i] = Wire.read();
    }
    passNr = Wire.read();
    bedrag = (highbyte << 8) | lowbyte;
  }
  money = true;
  // printReceipt = 1;
}


// de laaste nummers van de IBAN naar een int onzetten
void array2Lastnum() {
  for (int i = 0; i < 6; i++) {
    lastnum *= 10;
    lastnum += lastNumbers[i];
    lastNumbers[i] = NULL;
  }
}

void printBon() {
  printer.setSize('L');
  printer.println(F("OME-DUO"));

  printer.setSize('S');
  printer.feed(1);
  //  printer.inverseOn();
  //  printer.println(F(".                              ."));
  //  printer.inverseOff();

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
  if (minute <= 9) {
    printer.print("0");
  }
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
  printer.print(F("##########"));
  printer.println(lastnum);

  printer.boldOn();
  printer.justify('R');
  printer.println(F("Pas Nummer:"));
  printer.boldOff();
  printer.println(passNr);
  printer.feed(2);

  printer.justify('C');
  printer.setSize('M');
  printer.println(F("Opgenomen Bedrag"));
  printer.setSize('S');
  printer.setCharset(CHARSET_NORWAY);
  printer.write(0x24);
  printer.print(" ");
  printer.println(bedrag);
  printer.feed(2);

  printer.boldOn();
  printer.println(F("TOT ZIENS !"));
  printer.boldOff();
  printer.println();
  printer.feed(2);

  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}
void dispense() {
  int motor1brief = 50;
  int motor2brief = 50;
  int eenheid1 = 50;
  int eenheid2 = 10;
  float motor1tijd = 4.5;
  float motor2tijd = 4.5;
  int motor1count = 0;
  int motor2count = 0;
  while (bedrag - eenheid1 >= 0 && motor1brief - 1 >= 0) {
    motor1count++;
    bedrag = bedrag - eenheid1;
    motor1brief--;
  }
  while (bedrag - eenheid2 >= 0 && motor2brief - 1 >= 0) {
    motor2count++;
    bedrag = bedrag - eenheid2;
    motor2brief--;
  }
  Serial.println(motor1tijd * motor1count * 1000);
  while (motor1count > 0) {
    digitalWrite(motor1a, HIGH);
    delay(motor1tijd * 1000);
    digitalWrite(motor1a, LOW);
    delay(300);
    motor1count--;
  }
  while (motor2count > 0) {
    digitalWrite(motor2a, HIGH);
    delay(motor2tijd * 1000);
    digitalWrite(motor2a, LOW);
    delay(300);
    motor2count--;
  }
}

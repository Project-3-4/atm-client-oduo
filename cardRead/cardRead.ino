#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

/**
   SDA pin: 4
   SCK pin: 13
   MOSI pin: 11
   MISO pin: 12
   RQ pin: not used
   GND pin: GND
   RST pin: 3
   3.3V pin: 3.3V

*/
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {A0, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 2};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char userInput[4] {'x', 'x', 'x', 'x'};
String pinpas = "empty";
int tries = 2;
char opneemBedrag[4]= {'x', 'x', 'x', 'x'};

#define SS_PIN 4
#define RST_PIN 3
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  Wire.begin(2);
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

}
void loop() {
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  //Serial.println("Message : ");
  content.toUpperCase();
  if ((content.substring(1) == "B7 ED 7D 5A") || (content.substring(1) == "07 18 72 B4") ) //change here the UID of the card/cards that you want to give access
  {
    if (pinpas == "empty") {
      pinpas = content.substring(1);
      Serial.print(" Choose your pincode: ");
      char customKey = customKeypad.getKey();
      int test = 0;
      while (userInput[test] == 'x' && test < 4) {
        customKey = customKeypad.getKey();
        //Serial.println("test");
        if (customKey) {
          if (customKey == '#') {
            Serial.println(" ");
            Serial.println("backspace");
            Serial.println(" ");
            test--;
            userInput[test] = 'x';
            Serial.print(" Choose your pincode: ");
            int j = 0;
            while (j >= 0 && j <= 4) {
              if (userInput[j] != 'x') {
                Serial.print(userInput[j]);
                j++;
              }
              else {
                j = 5;
              }
            }
            //Serial.println(" ");
          }
          else if (customKey != "") {
            userInput[test] = customKey;
            Serial.print(customKey);
            delay(500);
            test++;
          }
          else {
            Serial.println("not valid key");
            delay(20);
          }
        }
      }
      Serial.println(" ");
      Serial.print(" You choose: ");
      for (int index = 0; index < 4; index++) {
        Serial.print(userInput[index]);
      }
      Serial.println(" ");
      delay(3000);

      //      Serial.print("Choose your pincode: ");
      //      while (Serial.available() == 0){
      //
      //      }
      //      userInput = Serial.parseInt();
      //      Serial.println(" ");
      //      Serial.print(" You choose: ");
      //      Serial.println(userInput);
    }
    else {
      int check = 0;
      int faults = 0;
      if (check == 0 && tries < 0) {
        Serial.println("CARD GOT DECLINED");
        delay(5000);
      }
      while (check == 0 && tries >= 0) {
        Serial.print("please enter your password: ");
        char userPass[4] = {'x', 'x', 'x', 'x'};
        char customKey = customKeypad.getKey();
        int i = 0;
        while (userPass[i] == 'x' && i < 4) {
          customKey = customKeypad.getKey();
          if (customKey) {
            if (customKey == '#') {
              Serial.println(" ");
              Serial.println("backspace");
              Serial.println(" ");
              i--;
              userPass[i] = 'x';
              Serial.print(" Enter your pincode: ");
              int j = 0;
              while (j >= 0 && j <= 4) {
                if (userPass[j] != 'x') {
                  Serial.print(userPass[j]);
                  j++;
                }
                else {
                  j = 5;
                }
              }
              //Serial.println(" ");
            }
            else if (customKey != " ") {
              userPass[i] = customKey;
              Serial.print(customKey);
              delay(500);
              i++;
            }
            else {
              Serial.println("not a valid key");
              delay(20);
            }
          }
        }
        Serial.println(" ");
        faults = 0;
        for (int i = 0; i < 4; i++) {
          if (userPass[i] != userInput[i]) {
            faults++;
          }
        }
        if (faults == 0) {
          Serial.println(" ");
          Serial.println("password correct");
          tries = 2;
          check = 1;
          Serial.println("Authorized access");
          Serial.println();
          digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
          int opnemen = 0;
          int getal = 0;
          Serial.println("Geld opnemen: A, afbreken: B");
          while (opnemen == 0) {
            customKey = customKeypad.getKey();
            if (customKey) {
              if (customKey == 'A') {
                int afbreken = 0;
                while (afbreken == 0) {
                  customKey = customKeypad.getKey();
                  if (customKey == 'B') {
                    afbreken = 1;
                    opnemen = 1;
                    Serial.println(" afbreken ");
                  }
                  else if (customKey != "") {
                    if (getal < 5) {
                      opneemBedrag[getal] = customKey;
                      getal++;
                    }
                  }
                }
              }
              else if (customKey == 'B') {
                opnemen = 1;
              }
            }
          }
          delay(5000);
          return;
        }
        else {
          Serial.println("password incorrent");
          Serial.print("tries remaining: ");
          Serial.print(tries);
          Serial.println(" ");
          tries--;
          if (tries < 0) {
            return;
          }
        }
      }
    }
  }

  else   {
    Serial.println(" Pass not recognized ");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(5000);
  }
}

void requestEvent() {
  Wire.write(opneemBedrag[4]);
}

#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

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
char opneemBedrag[4] = {'x', 'x', 'x', 'x'};
int opneemArray[4];
int16_t bedrag;
int16_t saldo = 9999;

#define SS_PIN 4
#define RST_PIN 3
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  Wire.begin();
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  Serial.println("Approximate your card to the reader...");
}
void loop() {
  bedrag = 0;
  // Serial.println("Approximate your card to the reader...");
  // Serial.println();
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
      Serial.print(" You chose: ");
      for (int index = 0; index < 4; index++) {
        Serial.print(userInput[index]);
      }
      Serial.println(" ");
      delay(3000);
      Serial.println("Approximate your card to the reader...");
    }
    else {
      int check = 0;
      int faults = 0;
      if (check == 0 && tries < 0) {
        Serial.println("CARD GOT DECLINED");
        delay(5000);
        Serial.println("Approximate your card to the reader...");
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
          Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
          while (opnemen == 0) {
            customKey = customKeypad.getKey();
            if (customKey) {
              if (customKey == 'A') {
                int afbreken = 0;
                Serial.println(" ");
                Serial.println("Confirm: A, cancel: B");
                Serial.print("Enter an amount: ");
                while (afbreken == 0) {
                  customKey = customKeypad.getKey();
                  if (customKey == 'B') {
                    afbreken = 1;
                    // opnemen = 1;
                    Serial.println(" canceling ");
                    Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
                    for (int i = 0; i < 4; i++) {
                      opneemBedrag[i] = 'x';
                    }
                    getal = 0;
                    continue;
                  }
                  else if (customKey == 'A' && getal >= 4) {
                    afbreken = 1;
                    // opnemen = 1;
                    Serial.print("Money withdrawn: ");
                    for (int i = 0; i < 4; i++) {
                      if (opneemBedrag[i] != 'x') {
                        Serial.print(opneemBedrag[i]);
                      }
                    }
                    Serial.println(" ");
                    bonOpvragen();
                    array2Int();
                    saldo = saldo - bedrag;
                    for (int i = 0; i < 4; i++) {
                      opneemBedrag[i] = 'x';
                    }
                    getal = 0;
                    //Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
                  }
                  else if (customKey == 'A' && getal < 4) {
                    for (int i = 0; i < 4; i++) {
                      if (opneemBedrag[i] == 'x') {
                        for (int j = i; j >= 0; j--) {
                          opneemBedrag[j] = opneemBedrag[j - 1];
                          if (j - 1 > 0) {
                            opneemBedrag[j - 1] = 'x';
                          }
                        }
                        opneemBedrag[0] = '0';
                      }
                    }
                    for (int i = 0; i < 4; i++) {
                      Serial.println(opneemBedrag[i]);
                    }
                    afbreken = 1;
                    // opnemen = 1;
                    array2Int();
                    Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
                    continue;
                    //Serial.println("Not a valid value");
                  }
                  else if (customKey == '#') {
                    opneemBedrag[getal - 1] = 'x';
                    getal--;
                    Serial.println("Backspace");
                    Serial.print("Enter an amount: ");
                    int p = 0;
                    while (opneemBedrag[p] != 'x') {
                      Serial.print(opneemBedrag[p]);
                      p++;
                    }
                  }
                  else if (customKey != NULL) {
                    if (getal < 4) {
                      opneemBedrag[getal] = customKey;
                      getal++;
                      Serial.print(customKey);
                    }
                    if (getal >= 4) {
                      Serial.println(" ");
                      Serial.println("Confirm: A, Cancel: B");
                    }
                  }
                }
              }
              else if (customKey == 'D') {
                opnemen = 1;
                Serial.println(" canceling ");
              }
              else if (customKey == 'B') {
                bedrag = 50;
                opnemen = 1;
                Serial.print("Money withdrawn: ");
                Serial.println(bedrag);
                Serial.println(" ");
                saldo = saldo - bedrag;
              }
              else if (customKey == 'C') {
                Serial.print("Balance is: ");
                Serial.println(saldo);
                // opnemen = 1;
                Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
                continue;
              }
            }
          }
          delay(5000);
          Serial.println("Approximate your card to the reader...");
          return;
        }
        else {
          Serial.println("password incorrent");
          Serial.print("tries remaining: ");
          Serial.print(tries);
          Serial.println(" ");
          tries--;
          if (tries < 0) {
            Serial.println("CARD GOT DECLINED");
            Serial.println(" ");
            Serial.println("Approximate your card to the reader...");
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
void array2Int() {
  int temp;
  for (int i = 0; i < 4; i++) {
    if (opneemBedrag[i] != 'x') {
      temp = opneemBedrag[i] - '0';
      opneemArray[i] = temp;
    }
  }
  for (int i = 0; i < 4; i++) {
    bedrag *= 10;
    bedrag += opneemArray[i];
  }
}

void sendItem() {
  byte highbyte = bedrag >> 8; //shift right 8 bits, leaving only the 8 high bits.
  byte lowbyte = bedrag & 0xFF; //bitwise AND with 0xFF
  Wire.write(highbyte);
  Wire.write(lowbyte);
}

void bonOpvragen() {
  Serial.println("Bonnentje? A: ja, B: nee)");
  Serial.println(" ");
  while (1) {
    char customKey = customKeypad.getKey();
    if (customKey == 'A') {
      Serial.println(" Reciept is being printed ");
      Wire.beginTransmission(2);
      sendItem();
      Wire.endTransmission();
      Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
      return;
    }
    else if (customKey == 'B') {
      // Serial.println(" canceling ");
      Serial.println("Withdraw: A, Quick Transaction ($50): B, See Balance: C, Done: D");
      return;
    }
    else if (customKey == NULL) {
      continue;
    }
  }
}

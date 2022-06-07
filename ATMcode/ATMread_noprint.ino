/**
    TODO:
    Groenland code on pass: GLODUO0000123400 == 47 4C 4F 44 55 4F 30 30 30 30 31 32 33 34 30 30
    Serial connection with GUI
    regel 444
*/
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

#define ResetPin 3
#define SlaveSelectPin 4
MFRC522 mfrc522(SlaveSelectPin, ResetPin);
MFRC522::MIFARE_Key key; // Crytpokey for accessing blocks

#define DS3231_I2C_ADDRESS 0x68 // Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}
byte bcdToDec(byte val) { // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}

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

char customKey;
int userIndex = 0;
char userInput[4] {'x', 'x', 'x', 'x'};
char otherVal[3] {'x', 'x', 'x'};
char pinCode[4] {'1', '2', '3', '4'};
boolean confermedPass = false;
int tries = 3;
int faults = 0;
boolean confermedCode = false;
boolean passScanned = false;
boolean exitPass = false;
int saldo = 500;
int opneembedrag;
byte highbyte;
byte lowbyte;
int passNumber[2];
int passNr;
long int lastnum;
int lastNumbers[6];

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  Wire.begin();
  mfrc522.PCD_Init();

  // Set default Crypto key for the trailer block (factory default)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  enterPincode();
  if (exitPass == true || tries <= 0) {
    return;
  }
  if (faults > 0) {
    tries--;
    return;
  }
  else {
    confermedCode = true;
  }

  if (confermedCode == true) {
    passScanned = false;
    tries = 3;
    mainMenu();
  }
  else {
    return;
  }
}

// opneemBedrag in 2 delen splitsen om te kunnen versturen met I2C
void sendItem() {
  highbyte = NULL;
  lowbyte = NULL;
  highbyte = (opneembedrag >> 8); //shift right 8 bits, leaving only the 8 high bits.
  lowbyte = opneembedrag & 0xFF; //bitwise AND with 0xFF
}

// informatie versturen naar de bonnenprinter
void printBon() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  sendItem();
  Wire.beginTransmission(4);
  Wire.write(highbyte);
  Wire.write(lowbyte);
  Wire.write(minute);
  Wire.write(hour);
  Wire.write(dayOfMonth);
  Wire.write(month);
  Wire.write(year);
  for (int i = 0; i < 6; i++) {
    Wire.write(lastNumbers[i]);
  }
  Wire.write(passNr);
  Wire.endTransmission();
  quit();
}

void quit() {
  confermedPass = false;
  passScanned = false;
}

void bonOpvragen() {
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        Serial.println(customKey);
        othermenu = true;
        printBon();
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        othermenu = true;
        quit();
      }
    }
  }
}


void confirmWithdraw() {
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        Serial.println(customKey);
        othermenu = true;
        saldo = saldo - opneembedrag;
        bonOpvragen();
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        othermenu = true;
        withdraw();
      }
    }
  }
}

// de laaste nummers van de IBAN naar een int onzetten
void array2Lastnum() {
  for (int i = 0; i < 6; i++) {
    lastnum *= 10;
    lastnum += lastNumbers[i];
    lastNumbers[i] = NULL;
  }
}

// het passNr naar een int omzetten
void array2PassNr() {
  for (int i = 0; i < 2; i++) {
    passNr *= 10;
    passNr += passNumber[i];
    passNumber[i] = NULL;
  }
}

// het gekozen opneembedrag omzetten naar een int
void array2Int() {
  int opneemArray[3];
  int temp;
  int count = 0;
  for (int i = 2; i >= 0; i--) {
    if (otherVal[i] == 'x') {
      count++;
    }
    if (otherVal[i] != 'x') {
      temp = otherVal[i] - '0';
      opneemArray[i] = temp;
    }
  }
  opneembedrag = 0;
  for (int i = 0; i < 3 - count; i++) {
    opneembedrag *= 10;
    opneembedrag += opneemArray[i];
  }
}

void otherValue() {
  boolean othermenu = false;
  int invoerIndex = 0;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'B') {
        Serial.println(customKey);
        if (invoerIndex > 0) {
          invoerIndex --;
          otherVal[invoerIndex] = 'x';
          for (int i = 0; i <= invoerIndex; i++) {
            if (otherVal[i] != 'x') {
            }
          }
        }
      }
      else if (customKey == 'C') {
        Serial.println(customKey);
        othermenu = true;
        array2Int();
        if (opneembedrag >= 10 && opneembedrag <= 300) {
          for (int i = 0; i < 3; i++) {
            otherVal[i] = 'x';
          }
          confirmWithdraw();
        }
        else {
          for (int i = 0; i < 3; i++) {
            if (otherVal[i] != 'x') {
            }
          }
          othermenu = false;
        }
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        for (int i = 0; i < 3; i++) {
          otherVal[i] = 'x';
        }
        othermenu = true;
        mainMenu();
      }
      else if (customKey != NULL && customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != 'D' && customKey != '*' && customKey != '#') {
        Serial.println(customKey);
        if (invoerIndex < 3) {
          otherVal[invoerIndex] = customKey;
          delay(200);
          invoerIndex++;
        }
        if (invoerIndex == 3) {
        }
      }
    }
  }
}

void withdraw() {
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == '1') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 20;
        confirmWithdraw();
      }
      else if (customKey == '4') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 50;
        confirmWithdraw();
      }
      else if (customKey == '7') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 100;
        confirmWithdraw();
      }
      else if (customKey == '*') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 150;
        confirmWithdraw();
      }
      else if (customKey == 'A') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 200;
        confirmWithdraw();
      }
      else if (customKey == 'B') {
        Serial.println(customKey);
        othermenu = true;
        opneembedrag = 300;
        confirmWithdraw();
      }
      else if (customKey == 'C') {
        Serial.println(customKey);
        othermenu = true;
        otherValue();
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        othermenu = true;
        mainMenu();
      }
    }
  }
}

void seeBalance() {
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'A') {
        Serial.println(customKey);
        othermenu = true;
        withdraw();
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        othermenu = true;
        mainMenu();
      }
    }
  }
}

void quickTransaction() {
  opneembedrag = 70;
  confirmWithdraw();
}

void mainMenu() {
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'A') {
        Serial.println(customKey);
        othermenu = true;
        seeBalance();
      }
      else if (customKey == 'B') {
        Serial.println(customKey);
        othermenu = true;
        if (saldo >= 0) {
          withdraw();
        }
        else {
          othermenu = false;
        }
      }
      else if (customKey == 'C') {
        Serial.println(customKey);
        othermenu = true;
        if (saldo >= 0) {
          quickTransaction();
        }
        else {
          othermenu = false;
        }
      }
      else if (customKey == 'D') {
        Serial.println(customKey);
        othermenu = true;
        quit();
      }
    }
  }
}

// array leeg maken zodat er de volgende keer weer in geschreven kan worden
void clearPin() {
  for (int i = 0; i < 4; i++) {
    userInput[i] = 'x';
  }
}

void enterPincode() {
  if ( tries <= 0 ) {
    passScanned = false;
  }
  if (passScanned == false) {
    lookForCard();
  }
  else {
    confermedPass = true;
  }
  while (true) {
    if (confermedPass == true) {
      passScanned = true;
      userIndex = 0;
      while (userIndex < 4) {
        customKey = customKeypad.getKey();
        if (customKey) {
          if (customKey == 'B' && userIndex > 0) {
            Serial.println(customKey);
            backSpace();
          }
          else if (customKey == 'D') {
            Serial.println(customKey);
            exitPass = true;
            confermedPass = false;
            passScanned = false;
            clearPin();
            delay(3000);
            return;
          }
          else if (customKey != NULL && customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != 'D' && customKey != '*' && customKey != '#') {
            Serial.println(customKey);
            userInput[userIndex] = customKey;
            delay(200);
            userIndex++;
          }
          else {
            delay(20);
          }
          if (userIndex == 4) {
            faults = 0;
            boolean exitloop = false;
            while (exitloop == false) {
              customKey = customKeypad.getKey();
              if (customKey == 'C') {
                Serial.println(customKey);
                String data;
                while (Serial.available() <=0){}
                if (Serial.available() > 0) {
                  data = Serial.readStringUntil('\n');
                }
                if(data == "1"){
                  faults = 0;
                }
                else{
                  faults = 4;
                }
                
                //                for (int i = 0; i < 4; i++) {
                //                  if (userInput[i] != pinCode[i]) {
                //                    faults++;
                //                  }
                //                }
                exitPass = false;
                clearPin();
                return;
              }
              else if (customKey == 'D') {
                Serial.println(customKey);
                exitPass = true;
                confermedPass = false;
                passScanned = false;
                clearPin();
                delay(3000);
                return;
              }
              else if (customKey == 'B') {
                Serial.println(customKey);
                backSpace();
                exitloop = true;
              }
            }
          }
        }
      }
      return;
    }
    else {
      continue;
    }
  }
}

void lookForCard() {
  while (true) {
    confermedPass = false;
    // Reset the loop if no new card is presented
    if (!mfrc522.PICC_IsNewCardPresent()) continue;

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) continue;

    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      continue;
    }

    /*** Preparing data for the PICC ***/
    // To avoid damage to the PICC never write data to Sector #0
    // In this example we will write data to Sector #1
    // Covering Block #4 up to and including Block #7
    byte sector = 0;
    byte blockAddr = 1; // Sector #1, Block #0
    byte dataBlock[]  = {
      0x47, 0x4C, 0x4F, 0x44, //  G,  L,   O,  D,
      0x55, 0x4F, 0x30, 0x30, //  U,  O,   0,  0,
      0x30, 0x30, 0x31, 0x32, //  0,  0,   1,  2,
      0x33, 0x34, 0x30, 0x31  //  3,  4,   0,  1
    };
    byte trailerBlock = 3; // Last block in Sector #1
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    /*** Accessing Sector #0 ***/
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      continue;
    }


    /*** Reading data from Sector #0 ***/
    // Print the data in the sector to the Serial Monitor

    // Read data from the block we are interested in
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
    }

    // Show data present in block 1 (Sector #0, Block #1)
    dump_byte_array(buffer, 16);
    // de laaste getallen uit de IBAN aflezen voor op de bon
    char IBAN[17];
    for (int i = 0; i < 16; i++) {
      IBAN[i] = char(buffer[i]);
      if (i >= 14) {
        passNumber[i - 14] = (char(buffer[i])) - '0';
      }
      if (i >= 10) {
        lastNumbers[i - 10] = (char(buffer[i])) - '0';
      }
    }
    IBAN[16] = '\0';
    Serial.print("+");
    Serial.println(String(IBAN));
    confermedPass = true;
    array2PassNr();

    /*** Close Connection ***/
    mfrc522.PICC_HaltA(); // Halt the PICC before stopping the encrypted session.
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
    if (tries > 0) {}
    else {
      delay(3000);
      confermedPass = false;
      passScanned = false;
    }


    if (confermedPass == false) {
      continue;
    }
    else {
      return;
    }
  }
}

/*** Helper function to dump a byte array as hex values to Serial Monitor. ***/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
  }
}


void backSpace() {
  userIndex--;
  userInput[userIndex] = 'x';
  int i = 0;
  if (userIndex > 0) {
    while (i >= 0 && i <= 4) {
      if (userInput[i] != 'x') {
        i++;
      }
      else {
        i = 5;
      }
    }
  }
}

// uitlezen van data uit de RTC
void readDS3231time(byte * second,
                    byte * minute,
                    byte * hour,
                    byte * dayOfWeek,
                    byte * dayOfMonth,
                    byte * month,
                    byte * year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

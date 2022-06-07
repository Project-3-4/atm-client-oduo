/**
    TODO:
    reset tries when pressing button
    Groenland code on pass: GRODUO0000123400 == 47 4C 4F 44 55 4F 30 30 30 30 31 32 33 34 30 30
    I2C IBAN number
    passnumber --> hex to char array
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
MFRC522 mfrc522(SlaveSelectPin, ResetPin); // Create MFRC522 instance.
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
int passNr;
int lastnum;
int lastNumbers[6];

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  Wire.begin();       // Init I2C bus
  mfrc522.PCD_Init(); // Init MFRC522 Module

  // Set default Crypto key for the trailer block (factory default)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("WARNING: Data will be written to the PICC, in sector #0"));
  Serial.println("Approximate your card to the reader...");
}

void loop() {
  //  if(tries <= 0){
  //    passScanned = false;
  //    Serial.println("Pass denied");
  //    delay(2000);
  //    Serial.println("Approximate your card to the reader...");
  //  }
  enterPincode();
  if (exitPass == true || tries <= 0) {
    return;
  }
  if (faults > 0) {
    tries--;
    Serial.println("Incorrect pincode!");
    Serial.print("tries remaining: ");
    Serial.println(tries);
    //    delay(3000);
    //    Serial.println("Approximate your card to the reader...");
    return;
  }
  else {
    confermedCode = true;
  }

  if (confermedCode == true) {
    passScanned = false;
    tries = 3;
    array2passnum();
    mainMenu();
  }
  else {
    Serial.println(confermedCode);
    Serial.println(faults);
    Serial.println("test2");
    return;
  }
}


void sendItem() {
  highbyte = NULL;
  lowbyte = NULL;
  highbyte = (opneembedrag >> 8); //shift right 8 bits, leaving only the 8 high bits.
  lowbyte = opneembedrag & 0xFF; //bitwise AND with 0xFF
}

void printBon() {
  Serial.println("printing receipt");
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  sendItem();
  Serial.println(second);
  Serial.println(year);
  Wire.beginTransmission(4);
  Wire.write(highbyte);
  Wire.write(lowbyte);
  Wire.write(minute);
  Wire.write(hour);
  Wire.write(dayOfMonth);
  Wire.write(month);
  Wire.write(year);
  Wire.endTransmission();
  quit();
}

void quit() {
  Serial.println("thanks for using Ome DUO have a nice day");
  confermedPass = true;
  passScanned = false;
  Serial.println("Approximate your card to the reader...");
}
void bonOpvragen() {
  Serial.println("Would you like a repeipt? ");
  Serial.println("C: yes, D: no");
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        othermenu = true;
        Serial.println("please wait until de dispenser opens");
        Serial.println("you can withdraw the cash from the cash dispenser now");
        printBon();
      }
      else if (customKey == 'D') {
        othermenu = true;
        Serial.println("please wait until de dispenser opens");
        Serial.println("you can withdraw the cash from the cash dispenser now");
        quit();
      }
    }
  }
}
void confirmWithdraw() {
  Serial.print("Are you sure you would like to withdraw ");
  Serial.println(opneembedrag);
  Serial.println("C: yes, D: no");
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        othermenu = true;
        saldo = saldo - opneembedrag;
        bonOpvragen();
      }
      else if (customKey == 'D') {
        othermenu = true;
        withdraw();
      }
    }
  }
}

void array2passnum(){
  int tempArray[2];
  int temp;
  int count = 0;
  for(int i = 6; i >= 5; i--){
    temp  = lastNumbers[i];
    tempArray[i] = temp;
  }
  passNr = 0;
  for(int i = 0; i < 2; i++){
    passNr *= 10;
    passNr += temp;
  }
  Serial.println(passNr);
}


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
  Serial.println("What amount would you like to withdraw?");
  Serial.println("amount cant be: smaller than 10 or bigger than 300");
  Serial.println("B: undo, C: confirm, D: quit");
  Serial.print("amount: ");
  boolean othermenu = false;
  int invoerIndex = 0;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'B') {
        if (invoerIndex > 0) {
          invoerIndex --;
          otherVal[invoerIndex] = 'x';
          Serial.print("amount: ");
          for (int i = 0; i <= invoerIndex; i++) {
            if (otherVal[i] != 'x') {
              Serial.print(otherVal[i]);
            }
          }
        }
      }
      else if (customKey == 'C') {
        othermenu = true;
        array2Int();
        if (opneembedrag >= 10 && opneembedrag <= 300) {
          for (int i = 0; i < 3; i++) {
            otherVal[i] = 'x';
          }
          confirmWithdraw();
        }
        else {
          Serial.println(" ");
          Serial.println("amount cant be: smaller than 10 or bigger than 300");
          Serial.println("B: undo, C: confirm, D: quit");
          Serial.print("amount: ");
          for (int i = 0; i < 3; i++) {
            if (otherVal[i] != 'x') {
              Serial.print(otherVal[i]);
            }
          }
          othermenu = false;
        }
      }
      else if (customKey == 'D') {
        for (int i = 0; i < 3; i++) {
          otherVal[i] = 'x';
        }
        othermenu = true;
        mainMenu();
      }
      else if (customKey != NULL && customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != 'D' && customKey != '*' && customKey != '#') {
        otherVal[invoerIndex] = customKey;
        Serial.print(customKey);
        delay(200);
        invoerIndex++;
        if (invoerIndex == 3) {
          Serial.println(" ");
          Serial.println("C: confirm");
        }
      }
    }
  }
}
void withdraw() {
  Serial.println("pin Amount");
  Serial.println("1: 20 | A: 200");
  Serial.println("4: 50 | B: 300");
  Serial.println("7: 100 | C: other");
  Serial.println("*: 150 | D: return");
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == '1') {
        othermenu = true;
        opneembedrag = 20;
        confirmWithdraw();
      }
      else if (customKey == '4') {
        othermenu = true;
        opneembedrag = 50;
        confirmWithdraw();
      }
      else if (customKey == '7') {
        othermenu = true;
        opneembedrag = 100;
        confirmWithdraw();
      }
      else if (customKey == '*') {
        othermenu = true;
        opneembedrag = 150;
        confirmWithdraw();
      }
      else if (customKey == 'A') {
        othermenu = true;
        opneembedrag = 200;
        confirmWithdraw();
      }
      else if (customKey == 'B') {
        othermenu = true;
        opneembedrag = 300;
        confirmWithdraw();
      }
      else if (customKey == 'C') {
        othermenu = true;
        otherValue();
      }
      else if (customKey == 'D') {
        othermenu = true;
        mainMenu();
      }
    }
  }
}


void seeBalance() {
  Serial.print("Balance: ");
  Serial.println(saldo);
  Serial.println("A: withdraw, D: return");
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'A') {
        othermenu = true;
        withdraw();
      }
      else if (customKey == 'D') {
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
  Serial.println("A: Balance, B: Withdraw, C: Quick transaction, D: Quit");
  boolean othermenu = false;
  while (othermenu == false) {
    customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'A') {
        othermenu = true;
        seeBalance();
      }
      else if (customKey == 'B') {
        othermenu = true;
        if (saldo >= 0) {
          withdraw();
        }
        else {
          Serial.println("not enough money to withdraw");
          othermenu = false;
          Serial.println("A: Balance, B: Withdraw, C: Quick transaction, D: Quit");
        }
      }
      else if (customKey == 'C') {
        othermenu = true;
        if (saldo >= 0) {
          quickTransaction();
        }
        else {
          Serial.println("not enough money to withdraw");
          othermenu = false;
          Serial.println("A: Balance, B: Withdraw, C: Quick transaction, D: Quit");
        }
      }
      else if (customKey == 'D') {
        othermenu = true;
        quit();
      }
    }
  }
}

void clearPin() {
  for (int i = 0; i < 4; i++) {
    userInput[i] = 'x';
  }
}
void enterPincode() {
  if ( tries <= 0 ) {
    passScanned = false;
    Serial.println("Approximate your card to the reader...");
  }
  if (passScanned == false) {
    lookForCard();
  }
  else {
    confermedPass = true;
  }
  while (true) {
    Serial.println("B: Undo, D: Quit");
    if (confermedPass == true) {
      passScanned = true;
      Serial.print(" Enter your pincode: ");
      userIndex = 0;
      while (userIndex < 4) {
        customKey = customKeypad.getKey();
        if (customKey) {
          if (customKey == 'B' && userIndex > 0) {
            backSpace();
          }
          else if (customKey == 'D') {
            Serial.println("Quiting");
            exitPass = true;
            confermedPass = false;
            passScanned = false;
            clearPin();
            delay(3000);
            Serial.println("Approximate your card to the reader...");
            return;
          }
          else if (customKey != NULL && customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != 'D' && customKey != '*' && customKey != '#') {
            userInput[userIndex] = customKey;
            Serial.print(customKey);
            delay(200);
            userIndex++;
          }
          else {
            Serial.println("not valid key");
            delay(20);
          }
          if (userIndex == 4) {
            Serial.println(" ");
            Serial.println("C: Confirm, D: Quit");
            faults = 0;
            boolean exitloop = false;
            while (exitloop == false) {
              customKey = customKeypad.getKey();
              if (customKey == 'C') {
                for (int i = 0; i < 4; i++) {
                  if (userInput[i] != pinCode[i]) {
                    faults++;
                  }
                }
                exitPass = false;
                clearPin();
                return;
              }
              else if (customKey == 'D') {
                Serial.println("Quiting");
                exitPass = true;
                confermedPass = false;
                passScanned = false;
                clearPin();
                delay(3000);
                Serial.println("Approximate your card to the reader...");
                return;
              }
              else if (customKey == '#') {
                backSpace();
                exitloop = true;
              }
              //Serial.println(" Array Full");
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

    Serial.print(F("Card UID:")); dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();

    Serial.print(F("PICC type: ")); MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      continue;
    }

    /*** [3] Preparing data for the PICC ***/
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

    /*** [4] Accessing Sector #1 ***/
    Serial.println(F("Accessing Sector #1 ..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      continue;
    }


    /*** [5] Reading data from Sector #1 ***/
    // Print the data in the sector to the Serial Monitor
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    // Read data from the block we are interested in
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }

    // Show data present in block 4 (Sector #1, Block #0)
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
    Serial.println();


    /*** [6] Write data to block 1 (Sector #0, Block #1) ***/
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    /*** [7] Read data from block 1 (Sector #0, Block #1) ***/
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();


    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (buffer[i] == dataBlock[i])
        count++;
      if (i >= 9){
        lastNumbers[i-9] = dataBlock[i];
      }
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
      Serial.println(F("Success :-)"));
      confermedPass = true;
    } else {
      Serial.println(F("Failure, no match :-("));
      Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();

    // Dump the sector data
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    /*** [8] Close Connection ***/
    mfrc522.PICC_HaltA(); // Halt the PICC before stopping the encrypted session.
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD

    if (tries > 0) {}
    else {
      Serial.println("Pass Denied");
      delay(3000);
      Serial.println("Approximate your card to the reader...");
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
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void backSpace() {
  Serial.println(" ");
  Serial.println("backspace");
  Serial.println(" ");
  userIndex--;
  userInput[userIndex] = 'x';
  Serial.print(" Enter your pincode: ");
  int i = 0;
  if (userIndex > 0) {
    while (i >= 0 && i <= 4) {
      if (userInput[i] != 'x') {
        Serial.print(userInput[i]);
        i++;
      }
      else {
        i = 5;
      }
    }
  }
}

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year) {
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

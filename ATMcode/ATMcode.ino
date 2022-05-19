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
char pinCode[4] {'1', '2', '3', '4'};
boolean confermedPass = false;
int tries = 3;
int faults = 0;
boolean confermedCode = false;
boolean passScanned = false;
boolean exitPass = false;





void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 Module

  // Set default Crypto key for the trailer block (factory default)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("WARNING: Data will be written to the PICC, in sector #1"));
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
    Serial.println("Welcome");
  }

  if (confermedCode == true) {
    Serial.println("test1");
    passScanned = false;
  }
  else {
    Serial.println(confermedCode);
    Serial.println(faults);
    Serial.println("test2");
    return;
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
    Serial.println("#: Undo, B: Quit");
    if (confermedPass == true) {
      passScanned = true;
      Serial.print(" Enter your pincode: ");
      userIndex = 0;
      while (userIndex < 4) {
        customKey = customKeypad.getKey();
        if (customKey) {
          if (customKey == '#' && userIndex > 0) {
            backSpace();
          }
          else if (customKey == 'B') {
            Serial.println("Quiting");
            exitPass = true;
            confermedPass = false;
            passScanned = false;
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
            Serial.println("A: Confirm, B: Quit");
            faults = 0;
            boolean exitloop = false;
            while (exitloop == false) {
              customKey = customKeypad.getKey();
              if (customKey == 'A') {
                for (int i = 0; i < 4; i++) {
                  if (userInput[i] != pinCode[i]) {
                    faults++;
                  }
                }
                exitPass = false;
                return;
              }
              else if (customKey == 'B') {
                Serial.println("Quiting");
                exitPass = true;
                confermedPass = false;
                passScanned = false;
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
    byte sector = 1;
    byte blockAddr = 4; // Sector #1, Block #0
    byte dataBlock[]  = {
      0xE5, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
      0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
      0x09, 0x0a, 0xff, 0x0b, //  9, 10, 255, 11,
      0x0c, 0x0d, 0x0e, 0x0f  // 12, 13, 14, 15
    };
    byte trailerBlock = 7; // Last block in Sector #1
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


    /*** [6] Write data to block 4 (Sector #1, Block #0) ***/
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    /*** [7] Read data from block 4 (Sector #1, Block #0) ***/
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

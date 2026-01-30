#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte dataBlocks[] = {4, 5, 6, 8, 9};  // Avoid trailer block 7
String fieldLabels[] = {"Product ID", "Name", "Quantity", "Value", "Timestamp"};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Initialize key to default (FF FF FF FF FF FF)
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  Serial.println("Place RFID tag to read...");
}

void loop() {
  // Wait for a new tag
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  Serial.println("\n=== Reading RFID Tag Data ===");

  for (int i = 0; i < 5; i++) {
    byte buffer[18];
    byte block = dataBlocks[i];
    byte length = 18;

    // Authenticate block
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.print("Auth failed at block "); Serial.println(block);
      continue;
    }

    // Read block
    if (rfid.MIFARE_Read(block, buffer, &length) != MFRC522::STATUS_OK) {
      Serial.print("Read failed at block "); Serial.println(block);
      continue;
    }

    // Convert buffer to String
    String data = "";
    for (int j = 0; j < 16; j++) {
      if (buffer[j] != 0x00) data += (char)buffer[j];
    }

    Serial.print(fieldLabels[i]); Serial.print(": ");
    Serial.println(data);
  }

  Serial.println("==============================");

  // Halt and stop encryption
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}

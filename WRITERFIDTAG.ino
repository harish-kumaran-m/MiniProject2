#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte dataBlocks[] = {4, 5, 6, 8, 9};  // Avoid sector trailers (e.g., block 7 is a trailer)
String fieldLabels[] = {"Product ID", "Name", "Quantity", "Value", "Timestamp"};
String fieldValues[5];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Set default key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  Serial.println("Enter Product details in CSV (ProductID,Name,Qty,Value,Timestamp):");
}

void loop() {
  // Wait for card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  Serial.println("Tag detected. Waiting for data via Serial...");

  while (Serial.available() == 0); // wait for input
  String input = Serial.readStringUntil('\n');
  input.trim();

  // Split CSV into field values
  int index = 0;
  while (input.length() > 0 && index < 5) {
    int commaIndex = input.indexOf(',');
    if (commaIndex == -1) {
      fieldValues[index++] = input;
      break;
    }
    fieldValues[index++] = input.substring(0, commaIndex);
    input = input.substring(commaIndex + 1);
  }

  // Write to blocks
  for (int i = 0; i < 5; i++) {
    byte block = dataBlocks[i];
    String data = fieldValues[i];
    data += "                ";  // pad
    data = data.substring(0, 16); // trim to 16 chars

    byte buffer[16];
    for (int j = 0; j < 16; j++) buffer[j] = data[j];

    // Authenticate
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.print("Auth failed at block "); Serial.println(block);
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return;
    }

    // Write
    if (rfid.MIFARE_Write(block, buffer, 16) != MFRC522::STATUS_OK) {
      Serial.print("Write failed at block "); Serial.println(block);
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return;
    }

    Serial.print("Wrote "); Serial.print(fieldLabels[i]);
    Serial.print(" to block "); Serial.println(block);
  }

  Serial.println("✅ Write Complete!");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}

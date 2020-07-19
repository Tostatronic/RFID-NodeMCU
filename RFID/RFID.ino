#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         5           // (D1)Configurable, see typical pin layout above
#define SS_PIN          4          // (D2) Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

char c;

void setup() {
  Serial.begin(9600);        // Initialize serial communications with the PC
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card
}

void loop()
{
  Serial.println("1. Escribir");
  Serial.println("2. Leer");
  while (!Serial.available()) {
    delay(50);
  }
  c = Serial.read();
  switch (c)
  {
    case '1':
      Escribir();
      break;
    case '2':
      Leer();
      break;
  }
}
void Escribir()
{
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory. 13.56mhz
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(20000L) ;     // Esperar 20 segundos hasta que el usuario ingrese el dato solicitado
  // Preguntar el codigo
  Serial.println(F("Escriba el codigo y finalice con #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ; // Lee la info introducida
  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // Rellena los espacios sin ocupar con "espacios" 54321#... - 543216#

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}
void Leer()
{
  // Pepaara la llave de acceso - por defecto viene seteadas con FFFFFFFFFFFFh directaente desde la fabrica.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reincia el Loop en caso de no encontrar una tarjeta presente.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Tarjeta Detectada:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card


  //---------------------------------------- Obtener Codigo
  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }


  //----------------------------------------

  Serial.println(F("\n**End Reading**\n"));

  delay(1000); //Cambiar el Valor si queremos leer tarjetas mas rapido.

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

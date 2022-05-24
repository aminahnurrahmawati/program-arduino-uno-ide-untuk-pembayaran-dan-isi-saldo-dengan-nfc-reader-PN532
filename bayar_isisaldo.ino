#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <PN532_I2C.h>// terhubung ke pn532 dengan sambungan I2C (bisa dengan spi atau hsu)
#include <PN532.h>
PN532_I2C pn532i2c(Wire); 
PN532 nfc(pn532i2c);  

char customKey ;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11,10, 9, 8};
byte colPins[COLS] = { 7, 6, 5, 4};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);  
  
long saldo;
long saldo_paling_akhir;
String input;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");
  lcd.init();
  lcd.backlight();
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  nfc.SAMConfig();  // configure board to read RFID tags
  
}
// saya menggunakan keypad untuk memilih nominal saldo yang ingin di top-up, begitupun untuk memilih menu bayar dan cek saldo
// tekan A => isi saldo 50k
// tekan B => isi saldo 25k
//tekan C => membayar
//tekan D=> cek saldo
// data saldo disimpan ke kartu di blok 1 sector 4 (bisa disimpan di sektor manapun)
// karna satu blok maksimal nilai decimal 255 (16 byte) ,maka maksimal saldo yang bisa ditop-up adalah 255k
// nilai saldo untuk di-print: nilai_decimal*1000


void loop(void) {
        customKey = customKeypad.getKey();
      if (customKey=='A'){
      lcd.print("Anda Memilih saldo 50k");
      isi_saldo();
      long saldo_akhir = saldo*1000 ;
      saldo_paling_akhir += saldo_akhir;
      lcd.clear();
      lcd.print("saldo anda adalah");
      lcd.print(saldo_akhir);
      delay(500);
      lcd.clear();
        }

       if (customKey=='B'){
      lcd.print("Anda Memilih saldo 25k");
     // int saldo_awal = 25 ;
     // saldo += saldo_awal;
      isi_saldo();
     // long saldo_akhir = saldo*1000 ;
      //saldo_paling_akhir += saldo_akhir;
      lcd.clear();
      lcd.print("saldo anda adalah");
//      lcd.print(saldo_akhir);
      delay(500);
      lcd.clear();
         
        }

        if (customKey=='C'){
        lcd.print("bayar");
        bayar();
        int saldo_akhir = saldo*1000 ;
        lcd.print("saldo anda adalah");
        lcd.print(saldo_akhir);
        delay(1000);
        lcd.clear();
        } 
          if (customKey=='D'){
      lcd.print("saldo anda adalah");
         cek_saldo();
         delay(1000);
         lcd.clear();
        }
   
  }

  void isi_saldo(){
    
   uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
//  digit = 2;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength); // mendeteksi nfc reader
   
  if (success) {
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
   // saldo -= 5;
    

    
    if (uidLength == 4)
    {
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya); //mendeteksi blok 0 dan sektor 4
    
      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
        lcd.clear();

        //saldo-=25 ;

        byte data[16] = {saldo , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

        success = nfc.mifareclassic_ReadDataBlock(4, data); //baca data yang di blok

       long saldo_akhir = data[0]+25; //menambahkan saldo (menambahkan data yanga ada pada blok)

        byte buffer[16] = {saldo_akhir , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ; // nilai data blok 0 yang terbaru (saldo_akhir)

        success = nfc.mifareclassic_WriteDataBlock (4, buffer);// menulis ulang data blok
        success = nfc.mifareclassic_ReadDataBlock(4, buffer);
        
    
        if (success)
        {
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 4:");
          nfc.PrintHexChar(data, 16);
          Serial.print(data[0]);
          lcd.clear();
          lcd.print(buffer[0]);
          lcd.print("000");
          Serial.println("");
      
          // Wait a bit before reading the card again
          delay(1000);
        }
      }
    }
  }
    }

  void bayar(){
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
//  digit = 2;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
   
  if (success) {
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
   // saldo -= 5;
    

    
    if (uidLength == 4)
    {
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
    
      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
        lcd.clear();


        byte data[16] = {saldo , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

        success = nfc.mifareclassic_ReadDataBlock(4, data);

       long saldo_akhir = data[0]-25;// karna membayar maka data blok 0 dikurangi dengan nilai harga

        byte buffer[16] = {saldo_akhir , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

        success = nfc.mifareclassic_WriteDataBlock (4, buffer);
        success = nfc.mifareclassic_ReadDataBlock(4, buffer);
        
        

        
        

    
        if (success)
        {
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 4:");
          nfc.PrintHexChar(data, 16);
          Serial.print(data[0]);
          lcd.clear();
          lcd.print(buffer[0]);
          lcd.print("000");
          Serial.println("");
      
          // Wait a bit before reading the card again
          delay(1000);
        }
      }
    }
  }
      
      }

      void cek_saldo(){
         uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
//  digit = 2;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
   
  if (success) {
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    
    if (uidLength == 4)
    {
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
    
      if (success)
      {
        byte data[16] = {saldo , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;
    
        success = nfc.mifareclassic_ReadDataBlock(4, data); //karna hanya ingin mengecek saldo maka perintah yang ditulis cukup pembacaan data blok

        if (success)
        {
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 4:");
          nfc.PrintHexChar(data, 16);
          Serial.print(data[0]);
          lcd.clear();
          lcd.print(data[0]);//print nilai pada data blok (saldo)
          lcd.print("000");
          Serial.println("");
      
          // Wait a bit before reading the card again
          delay(1000);
        }
      }
    }
  }
        
        }
  

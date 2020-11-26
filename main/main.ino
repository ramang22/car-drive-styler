#include "U8glib.h"
#include<Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// PIN setup for display
#define EN 13
#define RW 12
#define RS 11

// gryo act setup
const int MPU = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

// init display
U8GLIB_ST7920_128X64_1X lcd(EN, RW, RS);

// The TinyGPS++ object
TinyGPSPlus gps;

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The serial connection to the GPS device
SoftwareSerial ss(TXPin,RXPin);

long int refresh = 0;

void setup(void) {

  //display setup
  // whitecolor setup
  if ( lcd.getMode() == U8G_MODE_R3G3B2 ) {
    lcd.setColorIndex(255);
  }
  // brightness
  else if ( lcd.getMode() == U8G_MODE_GRAY2BIT ) {
    lcd.setColorIndex(3);
  }
  else if ( lcd.getMode() == U8G_MODE_BW ) {
    lcd.setColorIndex(1);
  }
  // rotate display
  // lcd.setRot180();

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  ss.begin(GPSBaud);


}

void loop(void) {

  if (millis() - refresh > 100) {
    lcd.firstPage();
    do {
      printTest();
    } while ( lcd.nextPage() );
    refresh = millis();
  }


  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = "); Serial.println(AcZ);

  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.println(gps.satellites.value());

  
if (ss.available() > 0){
    Serial.println(gps.satellites.value());
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
}
delay(333);
}
void printTest(void) {
  lcd.setFont(u8g_font_unifont);
  // nastavení pozice výpisu v pixelech
  // souřadnice jsou ve tvaru x, y
  // souřadnice 0, 0 je v levém horní rohu
  // OLED displeje, maximum je 128, 64
  lcd.setPrintPos(0, 10);
  // výpis textu na zadanou souřadnici
  lcd.print("Latitude");
  lcd.setPrintPos(0, 25);
  lcd.print("arduino-shop.cz");
  lcd.setPrintPos(0, 40);
  lcd.print("Cas od zapnuti:");
  lcd.setPrintPos(40, 55);
  lcd.print(millis()/1000);
  lcd.print(" vterin");
  
}

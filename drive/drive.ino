#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <U8g2lib.h>
#include<Wire.h>


// GPS PORTS
#define TX 3
#define RX 2
// PIN setup for display
#define EN 10
#define RW 9
#define RS 8
//gyroo and acc errors
#define ERRX 0
#define ERRY 0
#define ERRZ 0
#define GERRX -1.57
#define GERRY -1.07
#define GERRZ 0.53

// init display
U8G2_ST7920_128X64_1_SW_SPI lcd(U8G2_R0, EN, RW, RS);
int refresh = 0;

// gyro act setup
const int MPU = 0x68;
int16_t  AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

String GOOD = "GOOD";
String MEDIUM = "MEDIUM";
String BAD = "BAD";
String RACER = "RACER";
int type = 0;
SoftwareSerial swSerial(RX, TX);
TinyGPS gps;

void setup() {
  Serial.begin(9600);

  lcd.begin();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);

  swSerial.begin(9600);
}

void loop() {
  long gForce = 0;
  int speed = 0;
  char datumCas[32];
  bool newDataLoaded = false;

  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (swSerial.available()) {
      char c = swSerial.read();
      if (gps.encode(c)) {
        newDataLoaded = true;
      }
    }
  }
  if (newDataLoaded) {
    unsigned long gpsData;
    int year;
    byte month, day, hour, minute, second, miliSecond;
    speed = (gps.f_speed_kmph() == TinyGPS::GPS_INVALID_F_SPEED ? 0 : gps.f_speed_kmph());
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &miliSecond, &gpsData);
    sprintf(datumCas, "%02d/%02d/%02d %02d:%02d:%02d", month, day, year, hour, minute, second);
  }
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  AcX = (Wire.read() << 8 | Wire.read()) / 2048.0;
  AcX -= ERRX;
  AcY = (Wire.read() << 8 | Wire.read()) / 2048.0;
  AcY -= ERRY;
  AcZ = (Wire.read() << 8 | Wire.read()) / 2048.0;
  AcZ -= ERRZ;
  GyX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyX -= GERRX;
  GyY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyY -= GERRY;
  GyZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyZ -= GERRZ;

  gForce = sqrt(AcX * AcX + AcY * AcY + AcZ * AcZ);
  Serial.println(AcX);
  Serial.println(AcY);
  Serial.println(AcZ);
  if (millis() - refresh > 100) {
    lcd.firstPage();
    do {
      printTest(gForce, speed);
      //drawBitMap();
    } while ( lcd.nextPage() );
    refresh = millis();
  }
  if (speed > 100) {
    type = 4;
  } else if (gForce > 2) {
    type = 3;
  } else if (gForce <= 1) {
    type = 1;
  } else {
    type = 0;
  }
}


void printTest(unsigned long gForce, int speed) {
  lcd.setFont(u8g2_font_6x12_mf );
  lcd.setCursor(0, 7);
  lcd.print ("Speed : ");
  lcd.print(speed);
  lcd.print(" km/h");
  lcd.setCursor(0, 21);
  lcd.print("G-Force :");
  lcd.print(gForce);
  lcd.setCursor(0, 32);
  lcd.print("GX :");
  lcd.print(GyX);
  lcd.setCursor(0, 42);
  lcd.print("GY :");
  lcd.print(GyY);
  lcd.setCursor(0, 52);
  lcd.print("GZ :");
  lcd.print(GyZ);
  lcd.setCursor(0, 63);
  lcd.print("Style : ");
  if (type == 1) {
    lcd.print(GOOD);
  } else if (type == 2) {
    lcd.print(MEDIUM);
  } else if (type == 3) {
    lcd.print(BAD);
  } else {
    lcd.print(RACER);

  }
}

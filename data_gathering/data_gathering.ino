#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include<Wire.h>
#include <SD.h>


// GPS PORTS
#define TX 3
#define RX 2
// PIN setup for display
#define EN 10
#define RW 9
#define RS 8
#define TEXTFILE "aup1.csv"

// gyro act setup
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float AccErrorX, AccErrorY, AccErrorZ, GyroErrorX, GyroErrorY, GyroErrorZ;
int c = 0;

const int MPU = 0x68;


File myFile;

SoftwareSerial swSerial(RX, TX);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  if (!SD.begin(4)) {
    while (1);
  }
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  swSerial.begin(9600);

  Wire.beginTransmission(MPU);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission(true);

  calculate_IMU_error();

  File myFile = SD.open(TEXTFILE, FILE_WRITE);
  if (myFile) {
    Serial.print("done");
    myFile.println(F("date;gforce;speed;AccX;AccY;AccZ;GyroX;GyroY;GyroZ"));
    myFile.close();
  }
}

void loop() {
  TinyGPS gps;
  long gForce = 0;
  int speed = 0;
  char datumCas[32];
  bool newDataLoaded = false;

  File myFile = SD.open(TEXTFILE, FILE_WRITE);

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
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 2048.0;
    AccX -= AccErrorX;
    AccY = (Wire.read() << 8 | Wire.read()) / 2048.0;
    AccY -= AccErrorY;
    AccZ = (Wire.read() << 8 | Wire.read()) / 2048.0;
    AccZ -= AccErrorZ;
    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroX -= GyroErrorX;
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroY -= GyroErrorY;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ -= GyroErrorZ;

    gForce = sqrt(AccX * AccX + AccY * AccY + AccZ * AccZ);
    if (myFile) {
      Serial.print("done2");
      myFile.print(datumCas);
      myFile.print(F(";"));
      myFile.print(gForce);
      myFile.print(F(";"));
      myFile.print(speed);
      myFile.print(F(";"));
      myFile.print(AccX);
      myFile.print(F(";"));
      myFile.print(AccY);
      myFile.print(F(";"));
      myFile.print(AccZ);
      myFile.print(F(";"));
      myFile.print(GyroX);
      myFile.print(F(";"));
      myFile.print(GyroY);
      myFile.print(F(";"));
      myFile.print(GyroZ);
      myFile.close();
    }
  }
  delay(300);
}

void calculate_IMU_error() {
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 2048.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 2048.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 2048.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
}

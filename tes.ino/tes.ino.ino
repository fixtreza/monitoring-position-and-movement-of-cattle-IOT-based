#include <TinyGPS++.h> //Libary TinyGPS
#include <SoftwareSerial.h> //Libarary bawaan
#include "Wire.h"
#include "I2Cdev.h"
#include <ESP8266WiFi.h>
#include <MPU6050_tockn.h>


const char* ssid = "Galaxy A20s";
const char* password = "xxxxxxxxxxx";
const char* host = "34.194.185.249";

MPU6050 mpu6050(Wire);

long timer = 0;

// Choose two Arduino pins to use for software serial
int RXPin = 13; //Connect ke TX GPS
int TXPin = 15; //Connect ke RX GPS

int GPSBaud = 9600; //Biarin default

// Membuat objek TinyGPS++
TinyGPSPlus gps;

// Mmebuat koneksi serial dengan nama "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Jika koneksi berhasil, maka akan muncul address di serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Memulai koneksi serial pada baudrate 9600
  Serial.begin(9600);

  //Memulai koneksi serial dengan sensor
  gpsSerial.begin(GPSBaud);

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

}

void loop()
{
  mpu6050.update();

  //Membuat tampilan data ketika terdapat koneksi
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
      Serial.println("=======================================================");
  displayInfo();
  if (millis() - timer > 1000) {
    Serial.print("gyroX : "); Serial.print(mpu6050.getGyroX());
    Serial.print("  gyroY : "); Serial.print(mpu6050.getGyroY());
    Serial.print("  gyroZ : "); Serial.println(mpu6050.getGyroZ());

    Serial.print("gyroAngleX : "); Serial.print(mpu6050.getGyroAngleX());
    Serial.print("  gyroAngleY : "); Serial.print(mpu6050.getGyroAngleY());
    Serial.print("  gyroAngleZ : "); Serial.println(mpu6050.getGyroAngleZ());

    Serial.print("angleX : "); Serial.print(mpu6050.getAngleX());
    Serial.print("  angleY : "); Serial.print(mpu6050.getAngleY());
    Serial.print("  angleZ : "); Serial.println(mpu6050.getAngleZ());
    Serial.println("=======================================================");
    timer = millis();
  }

  // Jika dalam 5 detik tidak ada koneksi, maka akan muncul error "No GPS detected"
  // Periksa sambungan dan reset arduino
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while (true);
  }

  // Mengirimkan ke alamat host dengan port 80 -----------------------------------
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection Failed");
    return;
  }
  String latitude = String(gps.location.lat(), 6);
  String longitude = String(gps.location.lng(), 6);

  // Isi Konten yang dikirim adalah alamat ip si esp -----------------------------
  String url = "/isepi-aws/web-isepi/insert-data.php?Gx=";

  url += (mpu6050.getGyroX());
  url += "&Gy=";
  url += (mpu6050.getGyroY());
  url += "&Gz=";
  url += (mpu6050.getGyroZ());
  url += "&latitude=";
  url += String(gps.location.lat(), 6);
  url += "&longitude=";
  url += String(gps.location.lng(), 6);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // Mengirimkan Request ke Server -----------------------------------------------
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}

void displayInfo()
{
  if (gps.location.isValid())
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.println("Location: Not Available");
  }

  Serial.println();
  Serial.println();
  delay(1000);
}

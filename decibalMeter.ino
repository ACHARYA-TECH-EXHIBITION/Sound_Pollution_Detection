#include <SPI.h>

#include <String.h>

#include <ESP8266WiFi.h>

#include <NTPClient.h>

#include <WiFiUdp.h>

#include <FirebaseArduino.h>  //https://github.com/Rupakpoddar/ESP8266Firebase

#define WIFI_SSID "mardes home"
#define WIFI_PASSWORD "Prateek123@@"

#define projectID "warrior-bulls-default-rtdb" //Your Firebase Project ID; can be found in project settings.
#define FIREBASE_HOST "warrior-bulls-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "2JNTy9dXCRiq3Yyf78K9DqOhKZMzwnLeNIgZF5G3"

#define led 13

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float db = 0;
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  timeClient.begin();
  delay(2000);
}

void loop() {
  timeClient.update();
  unsigned long startMillis = millis(); // Start of sample window
  char result[50];
  long num = timeClient.getEpochTime();
  sprintf(result, "%ld", num);

  float peakToPeak = 0; // peak-to-peak level

  unsigned int signalMax = 0; //minimum value
  unsigned int signalMin = 1024; //maximum value

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(0); //get reading from microphone
    if (sample < 1024) // toss out spurious readings
    {
      if (sample > signalMax) {
        signalMax = sample; // save just the max levels
      } else if (sample < signalMin) {
        signalMin = sample; // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
  db = map(peakToPeak, 20, 900, 49.5, 90); //calibrate for deciBels

  if (db < 1024) {
    if (db > 70) {
      Serial.print(db); //write calibrated deciBels
      Serial.print(" dB Warning Danger !!"); //write units
      Serial.println();
      digitalWrite(led, HIGH);
    } else {
      Serial.print(db); //write calibrated deciBels
      Serial.print(" dB"); //write units
      Serial.println();
      digitalWrite(led, LOW);

    }

    char resultFinal[] = "RECORD/";
    strcat(resultFinal, result);
    //Serial.println(resultFinal);

    Firebase.setInt(resultFinal, db);

    Firebase.setInt("LIVE/VALUE", db);
  }

  delay(150);

}

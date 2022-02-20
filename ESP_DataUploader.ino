
// FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h" // Install Firebase ESP8266 library
#include <DHT.h> // Install DHT11 Library and Adafruit Unified Sensor Library
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial espSerial(16, 5);
#define FIREBASE_HOST                                                          \
  "firstproject-99bb2-default-rtdb.asia-southeast1.firebasedatabase.app/" // Without
                                                                          // http://
                                                                          // or
                                                                          // https://
                                                                          // schemes
#define FIREBASE_AUTH "yEf6VGsxnPXyfKp0B0sUeuWBQCxX0VqWRGEk6VPR"
#define WIFI_SSID "Room102"
#define WIFI_PASSWORD "Room@102_6"

#define DHTPIN 2 // Connect Data pin of DHT to D2
int led = 5;     // Connect LED to D5

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
String readString;
// Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseData ledData;

FirebaseJson json;
int US1_reading = 0;
int US2_reading = 0;
void refine_value(String tempString) {
  US1_reading = 0;
  US2_reading = 0;
  Serial.println("String working on : " + tempString);
  char tempChar[(tempString.length()) + 1];
  tempString.toCharArray(tempChar, (tempString.length()) + 1);
  int i;
  String returnString;
  int writedState = 0;
  if (tempChar[0] != 'D') {
    // its not our string
    Serial.println("unknow string!");
  } else if (tempChar[1] == '1') {
    Serial.println("Working fine...");
    for (i = 1; i < ((tempString.length()) + 1) && tempChar[i] != '~'; i++) {
      if (tempChar[i] == ':') {
        writedState++;
      }
      if (tempChar[i] >= '0' && tempChar[i] <= '9') {
        if (writedState == 1) {
          US1_reading += (int)tempChar[i];
          US1_reading *= 10;
        } else if (writedState == 2) {
          US2_reading += (int)tempChar[i];
          US2_reading *= 10;
        }
      }
    }
    US1_reading /= 10;
    US2_reading /= 10;
  }
}

void setup() {

  Serial.begin(115200);
  espSerial.begin(115200);
  dht.begin();
  pinMode(led, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}
int globle_HC_1 = 0;
int globle_HC_2 = 0;
void sensorUpdate() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C  ,"));
  Serial.print(f);
  Serial.println(F("F  "));

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/temperature", t)) {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/humidity", h)) {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  if (globle_HC_1 != US1_reading) {
    if (Firebase.setFloat(firebaseData, "/FirebaseIOT/Ultrasound_1",
                          US1_reading)) {
      Serial.println("U_Distance 1 updated!");
      globle_HC_1 = US1_reading;
    } else {
      Serial.println("U_Distance 1 not updated successfully!");
    }
  }
  if (globle_HC_2 != US2_reading) {
    if (Firebase.setFloat(firebaseData, "/FirebaseIOT/Ultrasound_2",
                          US2_reading)) {
      Serial.println("U_Distance 2 updated!");
      globle_HC_2 = US2_reading;
    } else {
      Serial.println("U_Distance 2 not updated successfully!");
    }
  }
}
bool dataEnterd = false;
void loop() {
  sensorUpdate();
  while (espSerial.available()) {
    char c = espSerial.read(); // gets one byte from serial buffer
    readString += c;        // makes the String readString
    delay(2); // slow looping to allow buffer to fill with next character
    dataEnterd = true;
  }
  if (dataEnterd) {
    refine_value(readString);
    dataEnterd = false;
  }

  if (Firebase.getString(ledData, "/FirebaseIOT/led")) {
    Serial.println(ledData.stringData());
    if (ledData.stringData() == "1") {
      digitalWrite(led, HIGH);
    } else if (ledData.stringData() == "0") {
      digitalWrite(led, LOW);
    }
  }
  delay(100);
}
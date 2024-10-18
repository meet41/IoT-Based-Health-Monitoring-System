#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include "AdafruitIO_WiFi.h"

// DHT11 Sensor
#define DHT_PIN D3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// LM35 Sensor
#define LM35_PIN A0

// Heart Rate Sensor
#define HEART_RATE_PIN D4

// Adafruit IO credentials
#define IO_USERNAME "meet1"
#define IO_KEY "aio_rUya88bq9brJ3vwrmfP3HOpFyCYD"

// Wi-Fi credentials
#define WIFI_SSID "your_SSID"
#define WIFI_PASS "your_PASSWORD"

// Firebase credentials
#define FIREBASE_HOST "https://health-monitor-37315-default-rtdb.firebaseio.com/"
const char* FIREBASE_AUTH = "x1pLGEim7Xvs4yWAUeMkmLnxNwTyxHO3CV9oTynq";

// Initialize Firebase and Adafruit IO
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Adafruit IO feeds
AdafruitIO_Feed *HeartRate = io.feed("HeartRate");
AdafruitIO_Feed *BloodPressure = io.feed("BloodPressure");
AdafruitIO_Feed *Temperature = io.feed("Temperature");
AdafruitIO_Feed *Humidity = io.feed("Humidity");

void setup() {
  Serial.begin(115200);
  Wire.begin();
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");

  io.connect();

  // Initialize Firebase
  firebaseConfig.database_url = FIREBASE_HOST;
  firebaseAuth.token.uid = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);

  // Verify Firebase connection
  if (Firebase.ready()) {
    Serial.println("Connected to Firebase");
  } else {
    Serial.println("Failed to connect to Firebase");
  }

  Serial.println("Connecting to Adafruit IO...");
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
}

void loop() {
  io.run();

  // Read Temperature and Humidity from DHT11
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperatureDHT) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature (DHT11): ");
    Serial.print(temperatureDHT);
    Serial.println(" *C");
    Temperature->save(temperatureDHT);
    if (Firebase.setFloat(firebaseData, "/temperature", temperatureDHT)) {
      Serial.println("Temperature data sent to Firebase");
    } else {
      Serial.println("Failed to send Temperature data to Firebase");
    }

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Humidity->save(humidity);
    if (Firebase.setFloat(firebaseData, "/humidity", humidity)) {
      Serial.println("Humidity data sent to Firebase");
    } else {
      Serial.println("Failed to send Humidity data to Firebase");
    }
  }

  // Read Body Temperature from LM35
  int lm35Value = analogRead(LM35_PIN);
  float temperatureLM35 = (lm35Value * 3.3) / 1024.0 * 100.0;

  Serial.print("LM35 Raw Value: ");
  Serial.println(lm35Value);
  Serial.print("Body Temperature (LM35): ");
  Serial.print(temperatureLM35);
  Serial.println(" *C");
  Temperature->save(temperatureLM35);
  if (Firebase.setFloat(firebaseData, "/body-temperature", temperatureLM35)) {
    Serial.println("Body Temperature data sent to Firebase");
  } else {
    Serial.println("Failed to send Body Temperature data to Firebase");
  }

  // Read Heart Rate - Ensure correct sensor library/method used
  int heartRateValue = digitalRead(HEART_RATE_PIN);  // This method may need to be adjusted based on the sensor
  Serial.print("Heart Rate Raw Value: ");
  Serial.println(heartRateValue);
  Serial.print("Heart Rate: ");
  Serial.println(heartRateValue);
  HeartRate->save(heartRateValue);
  if (Firebase.setInt(firebaseData, "/heart-rate", heartRateValue)) {
    Serial.println("Heart Rate data sent to Firebase");
  } else {
    Serial.println("Failed to send Heart Rate data to Firebase");
  }

  // Use default value for Blood Pressure
  float bloodPressure = 120.0;  // Default value
  Serial.print("Blood Pressure: ");
  Serial.print(bloodPressure);
  Serial.println(" mmHg");
  BloodPressure->save(bloodPressure);
  if (Firebase.setFloat(firebaseData, "/blood-pressure", bloodPressure)) {
    Serial.println("Blood Pressure data sent to Firebase");
  } else {
    Serial.println("Failed to send Blood Pressure data to Firebase");
  }

  delay(2000);  // Adjust the delay as needed
}

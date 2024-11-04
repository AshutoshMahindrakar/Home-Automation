#define BLYNK_TEMPLATE_ID "TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Home automation"
#define BLYNK_AUTH_TOKEN "AUTH_TOKEN"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "WIFI Name";
char pass[] = "WIFI Password";

char auth[] = BLYNK_AUTH_TOKEN;

// DHT11 Sensor
#define DHT_PIN D6
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Light and Fan pins
#define LIGHT_PIN D2
#define FAN_PIN D4

// Fan mode and state
bool fanAuto = false;
int fanState;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  
  // Wait for WiFi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  Blynk.begin(auth, ssid, pass);
  
  dht.begin();
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  Blynk.run();
  checkWiFi(); // Check WiFi connection

  // Read temperature only if in auto mode
  if (fanAuto) {
      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(2000); // Wait before retrying
      return;
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

            // Send data to Blynk Virtual Pins
      Blynk.virtualWrite(V3, temperature);
      Blynk.virtualWrite(V4, humidity);
    }

    // Fan automatic mode logic
    if (temperature > 25) {
      digitalWrite(FAN_PIN, HIGH); // Turn on fan
    } else {
      digitalWrite(FAN_PIN, LOW); // Turn off fan
    }
  }

  delay(1000); // Adjust delay as needed
}

// Function to check Wi-Fi connection
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, pass);
  }
}

// Blynk button handlers
BLYNK_WRITE(V0) {
  int lightState = param.asInt(); // Read the state of the light
  digitalWrite(LIGHT_PIN, lightState); // Turn on/off the light
}

BLYNK_WRITE(V1) {
  fanState = param.asInt(); // Read the state of the fan
  digitalWrite(FAN_PIN, fanState); // Turn on/off the fan
  fanAuto = false; // Set fan to manual mode when controlled via V1
}

BLYNK_WRITE(V2) {
  int fanMode = param.asInt(); // Read the fan mode
  if (fanMode == 1) {
    digitalWrite(FAN_PIN, LOW);
    fanAuto = true; // Set to auto mode
  } else {
    fanAuto = false; // Set to manual mode
    if (fanState == 1){
      digitalWrite(FAN_PIN, HIGH);
    }
    else {
      digitalWrite(FAN_PIN, LOW);
    }
  }
}

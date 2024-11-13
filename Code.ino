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
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Light and Fan pins
#define LIGHT_PIN D2
#define FAN_PIN D4

// Fan mode and state
bool fanAuto = false;
int fanState;
int fanMode;

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

  // Only read temperature and humidity if in auto mode
  if (fanAuto) {
      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

    // Check for failed readings
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
    if (temperature > 30) {
      digitalWrite(FAN_PIN, HIGH); // Turn on fan
    } else {
      digitalWrite(FAN_PIN, LOW); // Turn off fan
    }
  }

  // If the fan is in manual mode, don't send the temperature/humidity data
  else {
    // When fan is in manual mode, don't send data or send zero values (optional)
    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V4, 0);
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
  fanState = param.asInt();
  if (fanMode == 0) {
    if (fanState == 1) {
      digitalWrite(FAN_PIN, HIGH);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }
  }  
}

BLYNK_WRITE(V2) {
  int fanMode = param.asInt();
  if (fanMode == 1) {
    fanAuto = true;
    if (fanState == 1) {
      digitalWrite(FAN_PIN, LOW);
    }
  }

  else {
    fanAuto = false; 
    if (fanState == 1) {
      digitalWrite(FAN_PIN, HIGH);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }
  }
}

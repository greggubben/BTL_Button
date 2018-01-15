/*
 * Below The Line - Button
 * 
 * Connect to a WiFi Access Point
 * When button is pressed access the webpage
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

// Access Point Credentials
const char *ssid     = "below";
const char *password = "belowtheline";

IPAddress myIP;
IPAddress gwIP;

#define BUTTON_PIN D3   // Input Pin for the Button

int light_on = false;         // Current State of the LED
int buttonState = LOW;      // Current State of the Button
int lastButtonState = LOW;  // Last State of the Button
unsigned long lastDebounceTime = 0;       // the time the button state last switched
const unsigned long DEBOUNCE_DELAY = 50;  // min time the state must remain the same to register button press

void setup() {
  Serial.begin(115200);

  // Set up the button and internal LED
  light_on = false;
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  display_light();

  // Connect to private WiFi
  Serial.print("Connecting to access point \"");
  Serial.print(ssid);
  Serial.println("\" ...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  myIP = WiFi.localIP();
  gwIP = WiFi.gatewayIP();
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(myIP);
  Serial.print("GW address: ");
  Serial.println(gwIP);
}

void loop() {
  // read button state, HIGH when pressed, LOW when not
  int reading = digitalRead(BUTTON_PIN);

  // start the debounce timer
  if (reading != lastButtonState) {
    // reset the timer
    lastDebounceTime = millis();
  }

  // button has stopped bouncing - confirm state
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      // new state
      buttonState = reading;

      // Only make a change when the button is depressed
      if (buttonState == LOW) {
        light_on = change_light();
        display_light();
      }
      
    }
  }
  lastButtonState = reading;
}


bool change_light() {
  bool light_state = false;
  HTTPClient http;
  http.begin(gwIP.toString(), 80, "/");
  int httpCode = http.GET();
  Serial.print("HTTP Code = ");
  Serial.println(httpCode);
  if (httpCode == HTTP_CODE_OK) {
      String response_payload = http.getString();
      Serial.println(response_payload);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(response_payload);
      if (json.success()) {
        json.printTo(Serial);
        Serial.println("\nparsed json");
        light_state = json["light_state"];
      }
      else {
        Serial.println("failed to parse response");
      }
  }
  else {
    Serial.println("Error");
  }
  http.end();
  return light_state;
}
/*
 * Set the LED to ON or OFF based on light_on value
 */
void display_light() {
  digitalWrite(BUILTIN_LED, !light_on);
  if (light_on) {
    Serial.println("Light is ON");
  }
  else {
    Serial.println("Light is OFF");
  }
}

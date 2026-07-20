#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "CanTools-LED";
const char* AP_PASSWORD = "cantools123"; // WPA2 requires >= 8 chars

WebServer server(80);

// UART link to the LED controller Arduino (see 05_Mazda3CanHub)
#define ARDUINO_SERIAL    Serial2
#define ARDUINO_UART_RX   16
#define ARDUINO_UART_TX   17
#define ARDUINO_UART_BAUD 9600

const char PAGE[] =
#include "index.html"
;

void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleSetColor() {
  if (!server.hasArg("hex")) {
    server.send(400, "text/plain", "missing hex arg");
    return;
  }
  String hex = server.arg("hex");
  if (hex.length() != 6) {
    server.send(400, "text/plain", "hex must be 6 chars, e.g. ff0000");
    return;
  }

  long value = strtol(hex.c_str(), nullptr, 16);
  uint8_t r = (value >> 16) & 0xFF;
  uint8_t g = (value >> 8) & 0xFF;
  uint8_t b = value & 0xFF;

  ARDUINO_SERIAL.printf("%d,%d,%d\n", r, g, b);
  Serial.printf("color -> %d,%d,%d\n", r, g, b);

  server.send(200, "text/plain", "ok");
}

void setup() {
  Serial.begin(115200);
  ARDUINO_SERIAL.begin(ARDUINO_UART_BAUD, SERIAL_8N1, ARDUINO_UART_RX, ARDUINO_UART_TX);

  WiFi.softAP(AP_SSID, AP_PASSWORD);
  WiFi.setSleep(false); // AP modem-sleep otherwise delays responses enough to look like timeouts
  Serial.print("AP started, IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setColor", handleSetColor);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

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
#define ARDUINO_UART_BAUD 115200

const char PAGE[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>LED Color</title>
<style>
  body { font-family: sans-serif; text-align:center; background:#111; color:#eee; padding-top:40px; }
  input[type=color] { width:200px; height:120px; border:none; background:none; }
  #status { margin-top:20px; font-size:14px; color:#8f8; min-height:1.2em; }
</style>
</head>
<body>
  <h2>Door LED Color</h2>
  <input type="color" id="picker" value="#ff0000">
  <div id="status"></div>
<script>
  const picker = document.getElementById('picker');
  const status = document.getElementById('status');
  let pending = false, queued = null;

  function send(hex) {
    if (pending) { queued = hex; return; }
    pending = true;
    fetch('/setColor?hex=' + hex.substring(1))
      .then(r => { status.textContent = 'Sent ' + hex; })
      .catch(() => { status.textContent = 'Error sending color'; })
      .finally(() => {
        pending = false;
        if (queued) { const q = queued; queued = null; send(q); }
      });
  }

  picker.addEventListener('input', () => send(picker.value));
</script>
</body>
</html>
)rawliteral";

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

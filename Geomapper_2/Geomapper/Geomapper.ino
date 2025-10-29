#include "BluetoothSerial.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "map_html.h"
#include "mapping_struct.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth not enabled! Please run `make menuconfig` to enable it.
#endif

#define SWITCH_PIN 17
#define LED_PIN 2
#define right_motor_forward 27
#define right_motor_backward 14
#define right_motor_speed 26
#define left_motor_forward 16
#define left_motor_backward 4
#define left_motor_speed 13
#define TRIG_F 32
#define ECHO_F 39
#define TRIG_L 25
#define ECHO_L 34
#define TRIG_R 33
#define ECHO_R 36

// ========== GLOBALS ==========
BluetoothSerial SerialBT;
WebServer server(80);

// Make sonar globals (defined in sonar.ino) visible here
extern float front_distance;
extern float left_distance;
extern float right_distance;

float distanceForward = 0;
float distanceLeft = 0;
float distanceRight = 0;

// (and if you call this from mapping:)
void readAllSonars();  // prototype; implementation lives in sonar.ino


char x;
int spd;
float distance, duration;
volatile bool ledstate = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceTime = 500;

unsigned long lastMapScan = 0;

struct Scan {
  float f, l, r;
};
Scan scanData[120];
int scanCount = 0;

const char* ssid = "Blue";
const char* password = "11235813";

// ---------- Forward Declarations ----------

void bluetooth();
void motor(int a, int b);
void obstacle_avoidance();
float get_distance(int trigPin, int echoPin);
void readAllSonars();

// --- mapping externs/forwards (add) ---
extern void map_handle();
extern void map_startSession();
extern void map_cancelSession();
extern bool map_isEnabled();
extern bool map_isDone();
extern int  map_getCount();
extern const MapSample* map_data();
extern const struct AdvancedMapSample* map_getAdvancedData();

extern bool mappingEnabled;

// // We already use the sonar globals; ensure they are declared extern here:
// extern float front_distance;
// extern float left_distance;
// extern float right_distance;

// --- PWM constants defined here ---
const int freq = 500;
const int resolution = 8;

// ---------- Interrupt ----------
void IRAM_ATTR handleInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceTime) {
    ledstate = !ledstate;
    lastInterruptTime = currentTime;
  }
}

// ---------- WiFi + MDNS ----------
void connectToWiFi() {
  Serial.println("\n[Connecting to WiFi]");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected!");
  Serial.print("Local IP: "); Serial.println(WiFi.localIP());
  if (MDNS.begin("geomapper")) {
    Serial.println("mDNS responder started at http://geomapper.local/");
  } else Serial.println("Error starting mDNS");
}

// ---------- Web Server ----------
void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<h2>GeoMapper Active</h2><p><a href='/map'>Open Live Map</a></p><p>Use /status for JSON</p>");
  });

  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"front\":" + String(front_distance) + ",";
    json += "\"left\":" + String(left_distance) + ",";
    json += "\"right\":" + String(right_distance) + ",";
    json += "\"mapping\":" + String(mappingEnabled ? 1 : 0) + ",";
    json += "\"mode\":\"" + String(ledstate ? "autonomous" : "remote") + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // Serve the new mapping UI
server.on("/map", HTTP_GET, []() {
  extern const char* MAP_HTML; // from map_html.h below
  server.send_P(200, "text/html", MAP_HTML);
});

// Serve the sweep data (angles + distances)
server.on("/map_sweep", HTTP_GET, []() {
  int n = map_getCount();
  const MapSample* arr = map_data();
  String out = "{\"enabled\":" + String(map_isEnabled() ? 1 : 0)
             + ",\"done\":"    + String(map_isDone()    ? 1 : 0)
             + ",\"count\":"   + String(n) + ",\"items\":[";
  for (int i = 0; i < n; i++) {
    out += "{\"a\":" + String(arr[i].ang)
        +  ",\"f\":" + String(arr[i].f)
        +  ",\"l\":" + String(arr[i].l)
        +  ",\"r\":" + String(arr[i].r) + "}";
    if (i < n - 1) out += ",";
  }
  out += "]}";
  server.send(200, "application/json", out);
});


  server.on("/scan_history", HTTP_GET, []() {
    String out = "{\"count\":" + String(scanCount) + ",\"items\":[";
    for (int i = 0; i < scanCount; i++) {
      out += "{\"f\":" + String(scanData[i].f)
          +  ",\"l\":" + String(scanData[i].l)
          +  ",\"r\":" + String(scanData[i].r) + "}";
      if (i < scanCount - 1) out += ",";
    }
    out += "]}";
    server.send(200, "application/json", out);
  });

  server.begin();
  Serial.println("WebServer started.");
}


// ---------- Setup ----------
void setup() {
  pinMode(right_motor_forward, OUTPUT);
  pinMode(right_motor_backward, OUTPUT);
  pinMode(right_motor_speed, OUTPUT);
  pinMode(left_motor_forward, OUTPUT);
  pinMode(left_motor_backward, OUTPUT);
  pinMode(left_motor_speed, OUTPUT);
  pinMode(LED_PIN, OUTPUT); 
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  ledcAttach(left_motor_speed, freq, resolution);
  ledcAttach(right_motor_speed, freq, resolution);


  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), handleInterrupt, FALLING);
  Serial.begin(115200);
  SerialBT.begin();
  connectToWiFi();
  setupWebServer();
}

// ---------- Loop ----------
void loop() {
  server.handleClient();
  digitalWrite(LED_PIN, ledstate);

  // normal behavior untouched
  bluetooth();

  if (ledstate) { // Autonomous
    obstacle_avoidance();
  }

  map_handle();
}

/*
    Meshtastic send/receive client

    Connects to a Meshtastic node via WiFi or Serial (and maybe one day Bluetooth),
    and instructs it to send a text message every SEND_PERIOD milliseconds.
    The destination and channel to use can be specified.

    If the Meshtastic nodes receives a text message, it will call a callback function,
    which prints the message to the serial console.
*/

#include <Meshtastic.h>

// Pins to use for WiFi; these defaults are for an Adafruit Feather M0 WiFi.
#define WIFI_CS_PIN 8
#define WIFI_IRQ_PIN 7
#define WIFI_RESET_PIN 4
#define WIFI_ENABLE_PIN 2

// Pins to use for SoftwareSerial. Boards that don't use SoftwareSerial, and
// instead provide their own Serial1 connection through fixed pins
// will ignore these settings and use their own.
#ifndef SERIAL_RX_PIN
#define SERIAL_RX_PIN 16
#endif
#ifndef SERIAL_TX_PIN
#define SERIAL_TX_PIN 17
#endif
#ifndef BAUD_RATE
#define BAUD_RATE 57600
#endif

// Send a text message every this many seconds
#define SEND_PERIOD 60

uint32_t next_send_time = 0;
bool not_yet_connected = true;

// This callback function will be called whenever the radio connects to a node
void connected_callback(mt_node_t *node, mt_nr_progress_t progress) {
  if (not_yet_connected) 
    Serial.println("Connected to Meshtastic device!");
  Serial.print("progress: "); Serial.println(progress);
  not_yet_connected = false;
}

// This callback function will be called whenever the radio receives a text message
void text_message_callback(uint32_t from, const char* text) {
  // Do your own thing here. This example just prints the message to the serial console.
  Serial.print("Received a text message from ");
  Serial.print(from);
  Serial.print(": ");
  Serial.println(text);
}

void setup() {
  // Try for up to five seconds to find a serial port; if not, the show must go on
  Serial.begin(115200);
  while(true) {
    if (Serial) break;
    if (millis() > 5000) {
      Serial.print("Couldn't find a serial port after 5 seconds, continuing anyway");
      break;
    }
  }

  Serial.print("Booted Meshtastic send/receive client in ");

// Change to 1 to use a WiFi connection
#if 0
  #include "arduino_secrets.h"
  Serial.print("wifi");
  mt_wifi_init(WIFI_CS_PIN, WIFI_IRQ_PIN, WIFI_RESET_PIN, WIFI_ENABLE_PIN, WIFI_SSID, WIFI_PASS);
#else
  Serial.print("serial (RX="); Serial.print(SERIAL_RX_PIN);
  Serial.print(", TX="); Serial.print(SERIAL_TX_PIN); Serial.print(") ");
  mt_serial_init(SERIAL_RX_PIN, SERIAL_TX_PIN, BAUD_RATE);
#endif
  Serial.println(" mode");

  // Set to true if you want debug messages
  mt_set_debug(true);
  
  randomSeed(micros());

  // Initial connection to the Meshtastic device
  mt_request_node_report(connected_callback);

  // Register a callback function to be called whenever a text message is received
  set_text_message_callback(text_message_callback);

  Serial.println("Setup done. Waiting for meshtastic connection.");
}

void loop() {
  // Record the time that this loop began (in milliseconds since the device booted)
  uint32_t now = millis();

  // Run the Meshtastic loop, and see if it's able to send requests to the device yet
  bool can_send = mt_loop(now) && !not_yet_connected;

  // If we can send, and it's time to do so, send a text message and schedule the next one.
  if (can_send && now >= next_send_time) {
    
    // Change this to a specific node number if you want to send to just one node
    uint32_t dest = BROADCAST_ADDR; 
    // Change this to another index if you want to send on a different channel
    uint8_t channel_index = 0; 

    mt_send_text("Hello, world!", dest, channel_index);

    next_send_time = now + SEND_PERIOD * 1000;
  }
}

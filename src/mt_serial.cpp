#include "mt_internals.h"

#ifdef MT_SOFTWARESERIAL_SUPPORTED
  #include <SoftwareSerial.h>
  SoftwareSerial *serial;
#else
  #define serial (&Serial2)
#endif

void mt_serial_init(int8_t rx_pin, int8_t tx_pin, uint32_t baud) {
#ifdef MT_SOFTWARESERIAL_SUPPORTED
  serial = new SoftwareSerial(rx_pin, tx_pin);
#endif
  serial->begin(baud);
#ifndef MT_SOFTWARESERIAL_SUPPORTED
  serial->setPins(rx_pin, tx_pin);
#endif
  mt_wifi_mode = false;
  mt_serial_mode = true;
}

bool mt_serial_send_radio(const char * buf, size_t len) {
  size_t wrote = serial->write(buf, len);
  if (wrote == len) return true;

  if (mt_debugging) {
    Serial.print("Tried to send radio ");
    Serial.print(len);
    Serial.print(" but actually sent ");
    Serial.println(wrote);
  }

  return false;
}

bool mt_serial_loop() {
  return true;  // It's easy being a serial interface
}

size_t mt_serial_check_radio(char * buf, size_t space_left) {
  size_t bytes_read = 0;
  while (serial->available()) {
    char c = serial->read();
    *buf++ = c;
    if (++bytes_read >= space_left) {
      d("Serial overflow");
      break;
    }
  }
  if (mt_debugging && bytes_read > 0) {
    Serial.print("Received bytes len=");
    Serial.println(bytes_read);
  }
  return bytes_read;
}

/*
   RadioLib AX5043 AFSK AX.25 Transmit Example

   This example sends AX.25 messages using
   AX5043's AFSK modem.

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// AX5043 has the following connections:
// NSS pin:   7
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
AX5043 radio = new Module(7, 2, 9, 3);

// create AX.25 client instance using the FSK module
AX25Client ax25(&radio);

void setup() {
  Serial.begin(9600);

  // initialize SX1278
  Serial.print(F("[AX5043] Initializing ... "));
  // carrier frequency:           169.0 MHz
  int state = radio.beginAFSK(144.390e6);

  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while(true);
  }

  // initialize AX.25 client
  Serial.print(F("[AX.25] Initializing ... "));
  // source station callsign:     "N7LEM"
  // source station SSID:         0
  // preamble length:             8 bytes
  state = ax25.begin("N7LEM");
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while(true);
  }
}

void loop() {
  // send AX.25 unnumbered information frame
  Serial.print(F("[AX.25] Sending UI frame ... "));
  // destination station callsign:     "NJ7P"
  // destination station SSID:         0
  int state = ax25.transmit("Hello World!", "NJ7P");
  if (state == RADIOLIB_ERR_NONE) {
    // the packet was successfully transmitted
    Serial.println(F("success!"));

  } else {
    // some error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);

  }

  delay(5000);
}

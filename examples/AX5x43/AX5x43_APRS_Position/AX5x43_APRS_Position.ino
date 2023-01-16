/*
   RadioLib APRS Position Example

   This example sends APRS position reports 
   using AX5x43's FSK modem. The data is
   modulated as AFSK at 1200 baud using Bell 
   202 tones.

   DO NOT transmit in APRS bands unless
   you have a ham radio license!

   Other modules that can be used for APRS:
    - SX127x/RFM9x
    - RF69
    - SX1231
    - CC1101
    - nRF24
    - Si443x/RFM2x

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
// Uncomment below to use 16.368MHz XTAL
//#define AX5043_XTAL_16368KHz 1
#include <RadioLib.h>

// AX5043 has the following connections:
// NSS pin:   7
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
AX5043 radio = new Module(7, 2, 9, 3);

// create AFSK client instance using the FSK module
// pin 5 is connected to SX1278 DIO2
// AFSKClient audio(&radio, 5);

// create AX.25 client instance using the AFSK instance
AX25Client ax25(&radio);

// create APRS client isntance using the AX.25 client
APRSClient aprs(&ax25);

void setup() {
  Serial.begin(9600);

  // initialize AX5043
  // NOTE: moved to ISM band on purpose
  //       DO NOT transmit in APRS bands without ham radio license!
  Serial.print(F("[AX5043] Initializing ... "));
  int state = radio.beginAFSK(434.0);

  // when using one of the non-LoRa modules for AX.25
  // (RF69, CC1101, Si4432 etc.), use the basic begin() method
  // int state = radio.begin();

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

  // initialize APRS client
  Serial.print(F("[APRS] Initializing ... "));
  // symbol:                      '>' (car)
  state = aprs.begin('>');
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while(true);
  }
}

void loop() {
  Serial.print(F("[APRS] Sending position ... "));
  
  // send a location without message or timestamp
  int state = aprs.sendPosition("N0CALL", 0, "4911.67N", "01635.96E");
  delay(500);
  
  // send a location with message and without timestamp
  state |= aprs.sendPosition("N0CALL", 0, "4911.67N", "01635.96E", "I'm here!");
  delay(500);
  
  // send a location with message and timestamp
  state |= aprs.sendPosition("N0CALL", 0, "4911.67N", "01635.96E", "I'm here!", "093045z");
  delay(500);

  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }

  // wait one minute before transmitting again
  delay(60000);
}
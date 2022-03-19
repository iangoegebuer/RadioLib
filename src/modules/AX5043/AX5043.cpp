#include "AX5043.h"
#if !defined(RADIOLIB_EXCLUDE_AX5043)

AX5043::AX5043(Module* mod) : PhysicalLayer(RADIOLIB_AX5043_FREQUENCY_STEP_SIZE, RADIOLIB_AX5043_MAX_PACKET_LENGTH) {
  /*
    Constructor implementation MUST assign the provided "mod" pointer to the private "_mod" pointer.
  */
  _mod = mod;
}

Module* AX5043::getMod() {
  return(_mod);
}

int16_t AX5043::begin() {
  /*
    "begin" method implementation MUST call the "init" method with appropriate settings.
  */
  _mod->init();

  /*
    "begin" method SHOULD implement some sort of mechanism to verify the connection between Arduino and the module.

    For example, reading a version register
  */
}

int16_t AX5043::configModulation(uint8_t modulation) {
  int16_t state = _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_MODULATION, modulation);
  return state;
}

int16_t AX5043::getChipRevision() {
  return(_mod->SPIgetRegValue(RADIOLIB_AX5043_REG_SILICON_REVISION));
}

int16_t AX5043::beginAFSK(float freq) {
  /*
    "begin" method implementation MUST call the "init" method with appropriate settings.
  */
  _mod->init();
  _mod->pinMode(_mod->getIrq(), INPUT);
  _mod->pinMode(_mod->getGpio(), INPUT);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0xE7);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0x67);

  if(!getChipRevision()) {
    RADIOLIB_DEBUG_PRINTLN(F("No AX5043 found!"));
    _mod->term();
    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  } else {
    RADIOLIB_DEBUG_PRINTLN(getChipRevision());
  }

  int16_t state = configModulation(RADIOLIB_AX5043_MODULATION_AFSK);

  // Set to 3kHz deviation
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV, RADIOLIB_AX5043_FSKDEV_AFSK0);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+1, RADIOLIB_AX5043_FSKDEV_AFSK1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+2, RADIOLIB_AX5043_FSKDEV_AFSK2);
  
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK, RADIOLIB_AX5043_AFSKMARK1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK+1, RADIOLIB_AX5043_AFSKMARK0);

  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE, RADIOLIB_AX5043_AFSKSPACE1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE+1, RADIOLIB_AX5043_AFSKSPACE0);

  freq = freq * 1.048576;
  freq = freq + 0.5;
  uint32_t fr = (uint32_t)freq;
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA,   fr);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-1, fr >> 8);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-2, fr >> 16);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-3, fr >> 24);

  return fr;
}



int16_t AX5043::transmit(uint8_t* data, size_t len, uint8_t addr) {
  return 0;
}


int16_t AX5043::receive(uint8_t* data, size_t len) {
  return 0;
}


int16_t AX5043::standby() {
  return 0;
}

int16_t AX5043::transmitDirect(uint32_t frf = 0) {
  return 0;
}

int16_t AX5043::receiveDirect() {
  return 0;
}

int16_t AX5043::startTransmit(uint8_t* data, size_t len, uint8_t addr = 0) {
  return 0;
}

int16_t AX5043::readData(uint8_t* data, size_t len) {
  return 0;
}

int16_t AX5043::setFrequencyDeviation(float freqDev) {
  return 0;
}

size_t AX5043::getPacketLength(bool update = true) { 
  return 0;
}

int16_t AX5043::setEncoding(uint8_t encoding) { 
  return 0;
}

int16_t AX5043::setDataShaping(uint8_t sh) { 
  return 0;
}

void AX5043::readBit(RADIOLIB_PIN_TYPE pin) { 
  return;
}

void AX5043::setDirectAction(void (*func)(void)) {
  return;
}

uint8_t AX5043::randomByte() {
  return 0xA7; // It's random I swear!
}

#endif

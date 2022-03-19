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

int16_t AX5043::pllRanging() {
  uint8_t pll_loop_bak;
  uint8_t pll_cpi_bak;
  uint8_t pll_range = 0x18;

  // https://notblackmagic.com/bitsnpieces/ax5043//#rf-frequency-generation

  pll_loop_bak = _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PLLLOOP);
  pll_cpi_bak = _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PLLCPI);

  RADIOLIB_DEBUG_PRINTLN(pll_loop_bak, HEX);
  RADIOLIB_DEBUG_PRINTLN(pll_cpi_bak, HEX);

  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLVCODIV, 0x30);
  _mod->SPIsetRegValue(0xF10, 0x04); // XTAL Cap
  _mod->SPIsetRegValue(0xF11,                        0x00);
  _mod->SPIsetRegValue(0xF35,                        0x10);
  _mod->SPIsetRegValue(0x184, 0x0); // XTAL Cap
  _mod->SPIsetRegValue(0xF34,                        0x0F);

  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE,   0x05);  // AX5043_PWRSTATE_XTAL_ON

  while(!pll_range) {
    pll_range = _mod->SPIgetRegValue(0x01D);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(100);
  }

  pll_range = 0x18;
  
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA, 0x18);
  while(pll_range & 0x10) {
    pll_range = _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
  }
  RADIOLIB_DEBUG_PRINTLN(F("PLL ranging done"));
}

int16_t AX5043::beginAFSK(float freq) {
  /*
    "begin" method implementation MUST call the "init" method with appropriate settings.
  */
  _mod->init();
  _mod->pinMode(_mod->getIrq(), INPUT);
  _mod->pinMode(_mod->getGpio(), INPUT);

  // Reset Chip
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0xE7);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0x67);

  // Check if the chup exists
  if(!getChipRevision()) {
    RADIOLIB_DEBUG_PRINTLN(F("No AX5043 found!"));
    _mod->term();
    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  } else {
    RADIOLIB_DEBUG_PRINTLN(getChipRevision());
  }

  int16_t state = configModulation(RADIOLIB_AX5043_MODULATION_AFSK);

  // Set to 3kHz deviation
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV, RADIOLIB_AX5043_FSKDEV_AFSK2);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+1, RADIOLIB_AX5043_FSKDEV_AFSK1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+2, RADIOLIB_AX5043_FSKDEV_AFSK0);
  
  // Mark and space at 2200Hz and 1200Hz
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK, RADIOLIB_AX5043_AFSKMARK1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK+1, RADIOLIB_AX5043_AFSKMARK0);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE, RADIOLIB_AX5043_AFSKSPACE1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE+1, RADIOLIB_AX5043_AFSKSPACE0);

  // Set power to 15dB
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXPWRCOEFFB,   0xFF);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXPWRCOEFFB-1, 0x0F);

  // frequncy * 2^24 / 16MHz(xtal)
  freq = freq * 1.048576;
  // Round to nearest bit
  freq = freq + 0.5;
  // Convert to binary data
  uint32_t fr = (uint32_t)freq;
  fr = 0x0a900001;
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA,   fr);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-1, fr >> 8);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-2, fr >> 16);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-3, fr >> 24);

  RADIOLIB_DEBUG_PRINTLN(fr, HEX);

  pllRanging();

  return RADIOLIB_ERR_NONE;
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

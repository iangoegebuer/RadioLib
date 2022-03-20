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
  uint8_t pll_range = 0;

  // Thanks for a lot of help on the register mapping
  // https://notblackmagic.com/bitsnpieces/ax5043//#rf-frequency-generation

  // From AND9347-D.PDF:
  // Internal Loop Filter x2, BW = 200 kHz for ICP = 272 mA
  // Bypass external filter
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLLOOP, 0x0A);
  // Charge pump current set to 16*8.5uA
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLCPI, 0x10);

  // TODO: Support dividing these for lower frequencies
  // Essenitally, try to range. If ranging fails, double the 
  // frequency and set the RFDIV bits here. Then retry ranging
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLVCODIV, 0x30);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_F34,       RADIOLIB_AX5043_F34_NO_RFDIV);

  // Setup unnanmed power registers based on data sheet
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_F10, RADIOLIB_AX5043_F10_TCXO); 
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_F11, RADIOLIB_AX5043_F11_TCXO);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_F35, RADIOLIB_AX5043_F35_XTALDIV_1);

  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE,   0x67 | 0x05);  // AX5043_PWRSTATE_XTAL_ON

  // Wait for xtal to be running
  while(!pll_range) {
    pll_range = _mod->SPIgetRegValue(0x01D);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(100);
  }

  pll_range = 0x18;
  
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA, RADIOLIB_AX5043_REG_PLLRANGEA_VCORANGE_RST | RADIOLIB_AX5043_REG_PLLRANGEA_RNG_START);
  while(pll_range & RADIOLIB_AX5043_REG_PLLRANGEA_RNG_START) {
    pll_range = _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(100);
  }

  RADIOLIB_DEBUG_PRINTLN(F("PLL ranging done"));

  if (pll_range & 0x20) return RADIOLIB_ERR_RANGING_TIMEOUT;

  return RADIOLIB_ERR_NONE;
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
  
  // TX Rate
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE,   0xEB);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE-1, 0x4);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE-2, 0x0);
  
  // Mark and space at 2200Hz and 1200Hz
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK,   RADIOLIB_AX5043_AFSKMARK1);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK+1, RADIOLIB_AX5043_AFSKMARK0);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE,   RADIOLIB_AX5043_AFSKSPACE1);
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

  // From AND9347-D.PDF, It is strongly recommended to always set bit 0 to avoid spectral tones.
  fr |= 1;

  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA,   fr);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-1, fr >> 8);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-2, fr >> 16);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-3, fr >> 24);

  RADIOLIB_DEBUG_PRINTLN(fr, HEX);

  return pllRanging();
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

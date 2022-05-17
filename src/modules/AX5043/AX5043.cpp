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

uint16_t AX5043::waitForXtal() {
  uint8_t pll_range = 0;

  // Wait for xtal to be running
  while(!pll_range) {
    pll_range = _mod->SPIgetRegValue(0x01D);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(1);
  }
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

  waitForXtal();

  pll_range = 0x18;
  
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA, RADIOLIB_AX5043_REG_PLLRANGEA_VCORANGE_RST | 
                                                      RADIOLIB_AX5043_REG_PLLRANGEA_RNG_START);

  while(pll_range & RADIOLIB_AX5043_REG_PLLRANGEA_RNG_START) {
    pll_range = _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PLLRANGEA);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(100);
  }

  RADIOLIB_DEBUG_PRINTLN(F("PLL ranging done"));

  if (pll_range & 0x20) return RADIOLIB_ERR_RANGING_TIMEOUT;

  return RADIOLIB_ERR_NONE;
}

int16_t AX5043::beginAFSK(uint32_t freq, uint32_t txRate, uint32_t fskDev, uint16_t afskMark, uint16_t afskSpace) {
  /*
    "begin" method implementation MUST call the "init" method with appropriate settings.
  */
  _mod->init();
  _mod->pinMode(_mod->getIrq(), INPUT);
  _mod->pinMode(_mod->getGpio(), INPUT);

  // Reset Chip
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0xE7);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, 0x67);

  // Check if the chip exists
  if(!getChipRevision()) {
    RADIOLIB_DEBUG_PRINTLN(F("No AX5043 found!"));
    _mod->term();
    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  } else {
    RADIOLIB_DEBUG_PRINTLN(getChipRevision());
  }

  int16_t state = configModulation(RADIOLIB_AX5043_MODULATION_AFSK);

  // Set to 3kHz deviation
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV,    fskDev&0xff);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV-1, (fskDev>>8)&0xff);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV-2, (fskDev>>16)&0xff);
  
  // TX Rate
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE,    txRate&0xff);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE-1, (txRate>>8)&0xff);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXRATE-2, (txRate>>16)&0xff);

  // Mark and space at 2200Hz and 1200Hz
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK,     afskMark&0xFF);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK-1,  (afskSpace>>8)&0xFF);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE,    afskSpace);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE-1, (afskSpace>>8)&0xFF);

  // Set power to 15dB
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXPWRCOEFFB,   0xFF);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_TXPWRCOEFFB-1, 0x0F);

  // frequncy * 2^24 / 16MHz(xtal)
  // freq = freq * 1.048576;
  // // Round to nearest bit
  // freq = freq + 0.5;

  // From AND9347-D.PDF, It is strongly recommended to always set bit 0 to avoid spectral tones.
  freq |= 1;

  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA,    freq&0xFF);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-1, (freq >> 8)&0xFF);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-2, (freq >> 16)&0xFF);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-3, (freq >> 24)&0xFF);

  RADIOLIB_DEBUG_PRINTLN(fr, HEX);

  return pllRanging();
}



int16_t AX5043::transmit(uint8_t* data, size_t len, uint8_t addr) {
  RADIOLIB_DEBUG_PRINTLN(F("transmit called"));
  RADIOLIB_DEBUG_PRINTLN(len);
  RADIOLIB_DEBUG_PRINTLN(addr, HEX);
  
  // Clear the FIFO and switch to TX mode
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFOSTAT, RADIOLIB_AX5043_FIFOSTAT_CMD_CLEAR_FIFO);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE,  RADIOLIB_AX5043_PWRMODE_FULL_TX);


  while (!(_mod->SPIgetRegValue(RADIOLIB_AX5043_REG_PWRSTAT) & 0x08));

  // Write data to FIFO
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFODATA, RADIOLIB_AX5043_REG_FIFODATA_TYPE_DATA);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFODATA, len+1);  // Length + flag
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFODATA, 0x33);   // Raw+skip encoding | Packet start + packet end

  // Write packet
  for (int i = 0; i < len; i++) {
    // Our data is pre-inverted 
    _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFODATA, Module::flipBits(data[i]));
  }

  // Read Event reg to clear it
  _mod->SPIgetRegValue(RADIOLIB_AX5043_REG_RADIOEVENTREQ0);

  // Send the message!
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FIFOSTAT, RADIOLIB_AX5043_FIFOSTAT_CMD_COMMIT);

  // Set event mask to transmit done
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_RADIOEVENTMASK0, 0x01);
  // Set IRQ mask to PLLUNLOCK + Radio controller interrupt
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_IRQMASK0, 0x40);

  // Wait until TX is finished
  while(1)
  {
    // Transmit done?
    if(_mod->SPIgetRegValue(RADIOLIB_AX5043_REG_RADIOEVENTREQ0) & 0x01)
      break;
  }

  // Transmit is done power down
  // TODO: Switch to RX?
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_PWRMODE, RADIOLIB_AX5043_PWRMODE_POWERDOWN);

  return 0;
}


int16_t AX5043::receive(uint8_t* data, size_t len) {
  RADIOLIB_DEBUG_PRINTLN(F("receive called"));
  return 0;
}


int16_t AX5043::standby() {
  RADIOLIB_DEBUG_PRINTLN(F("standby called"));
  return 0;
}

int16_t AX5043::transmitDirect(uint32_t frf = 0) {
  RADIOLIB_DEBUG_PRINTLN(F("transmitDirect called"));
  return 0;
}

int16_t AX5043::receiveDirect() {
  RADIOLIB_DEBUG_PRINTLN(F("receiveDirect called"));
  return 0;
}

int16_t AX5043::startTransmit(uint8_t* data, size_t len, uint8_t addr = 0) {
  RADIOLIB_DEBUG_PRINTLN(F("startTransmit called"));
  return 0;
}

int16_t AX5043::readData(uint8_t* data, size_t len) {
  RADIOLIB_DEBUG_PRINTLN(F("readData called"));
  return 0;
}

int16_t AX5043::setFrequencyDeviation(float freqDev) {
  RADIOLIB_DEBUG_PRINTLN(F("setFrequencyDeviation called"));
  return 0;
}

size_t AX5043::getPacketLength(bool update = true) { 
  RADIOLIB_DEBUG_PRINTLN(F("getPacketLength called"));
  return 0;
}

int16_t AX5043::setEncoding(uint8_t encoding) { 
  RADIOLIB_DEBUG_PRINTLN(F("setEncoding called"));

  if (encoding == RADIOLIB_ENCODING_NRZ) {
    _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_ENCODING, RADIOLIB_AX5043_ENCODING_NRZ);
  }

  return 0;
}

int16_t AX5043::setDataShaping(uint8_t sh) { 
  RADIOLIB_DEBUG_PRINTLN(F("setDataShaping called"));
  return 0;
}

void AX5043::readBit(RADIOLIB_PIN_TYPE pin) { 
  RADIOLIB_DEBUG_PRINTLN(F("readBit called"));
  return;
}

void AX5043::setDirectAction(void (*func)(void)) {
  RADIOLIB_DEBUG_PRINTLN(F("setDirectAction called"));
  return;
}

uint8_t AX5043::randomByte() {
  RADIOLIB_DEBUG_PRINTLN(F("randomByte called"));
  return 0xA7; // It's random I swear!
}

#endif

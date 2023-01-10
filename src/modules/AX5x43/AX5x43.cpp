#include "AX5x43.h"
#if !defined(RADIOLIB_EXCLUDE_AX5X43)

AX5x43::AX5x43(Module* module) : PhysicalLayer(RADIOLIB_AX5X43_FREQUENCY_STEP_SIZE, RADIOLIB_AX5X43_MAX_PACKET_LENGTH) {
  mod = module;
}

Module* AX5x43::getMod() {
  return(mod);
}

int16_t AX5x43::begin(float frq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength) {
  // set module properties
  mod->SPIreadCommand |= RADIOLIB_AX5X43_SPI_LONG_ADDR_CMD;
  mod->SPIwriteCommand |= RADIOLIB_AX5X43_SPI_LONG_ADDR_CMD;
  mod->SPIaddrWidth = 16;
  mod->init();
  mod->pinMode(mod->getIrq(), INPUT);

  // Mode setting enable ref
  if(refEnabled) refEnabled = 1 << 5;
  // Mode setting enable crystal
  if(crystalEnabled) crystalEnabled = 1 << 6;

  // try to find the CC1101 chip
  uint8_t i = 0;
  bool flagFound = false;
  while((i < 10) && !flagFound) {
    int16_t version = getChipVersion();
    if(version == RADIOLIB_AX5X43_SILICONREV) {
      flagFound = true;
    } else {
      #if defined(RADIOLIB_DEBUG)
        RADIOLIB_DEBUG_PRINT(F("AX5x43 not found! ("));
        RADIOLIB_DEBUG_PRINT(i + 1);
        RADIOLIB_DEBUG_PRINT(F(" of 10 tries) RADIOLIB_AX5X43_REG_REVISION == "));

        char buffHex[7];
        sprintf(buffHex, "0x%04X", version);
        RADIOLIB_DEBUG_PRINT(buffHex);
        RADIOLIB_DEBUG_PRINT(F(", expected 0x0051"));
        RADIOLIB_DEBUG_PRINTLN();
      #endif
      mod->delay(10);
      i++;
    }
  }

  if(!flagFound) {
    RADIOLIB_DEBUG_PRINTLN(F("No AX5x43 found!"));
    mod->term();
    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  } else {
    RADIOLIB_DEBUG_PRINTLN(F("M\tAX5x43"));
  }

  // setting initial frequency to 0 will force setFrequency to perform VCO ranging
  freq = 0.0;

  // reset the module
  int16_t state = reset();
  RADIOLIB_ASSERT(state);

  // configure settings not accessible by API
  state = config();
  RADIOLIB_ASSERT(state);

  state = setFrequencyDeviation(freqDev);
  RADIOLIB_ASSERT(state);

  state = setBitRate(br);
  RADIOLIB_ASSERT(state);

  // configure publicly accessible settings
  state = setFrequency(frq);
  RADIOLIB_ASSERT(state);

  state = setPreambleLength(preambleLength);
  RADIOLIB_ASSERT(state);

  // TODO: Set power here properly
  state =  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_PWR_COEFF_A_0,    0xff);
  state =  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_PWR_COEFF_A_1,    0x02);

  state =  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_PWR_COEFF_B_0,    0xff);
  state =  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_PWR_COEFF_B_1, 0x02);

  return(state);
}

int16_t AX5x43::reset() {
  // set the reset bit - only check the MSB, since mode will be set to power down
  int16_t state = setMode(RADIOLIB_AX5X43_PWR_MODE_RESET_SET);
  RADIOLIB_ASSERT(state);

  // hold it there for a while
  mod->delayMicroseconds(100);

  // clear the reset bit
  state = setMode(RADIOLIB_AX5X43_PWR_MODE_RESET_CLEAR);
  RADIOLIB_ASSERT(state);

  // set power mode to power down, as called for by the datasheet
  state = setMode(RADIOLIB_AX5X43_PWR_MODE_POWER_DOWN);

  // hold it there for a while
  mod->delayMicroseconds(100);

  return(state);
}

int16_t AX5x43::standby() {
  // set RF switch (if present)
  mod->setRfSwitchState(LOW, LOW);

  return(setMode(RADIOLIB_AX5X43_PWR_MODE_FIFO_ON));
}

int16_t AX5x43::setFrequency(float frq) {
  return(setFrequency(frq, false));
}

float AX5x43::getCrystalFrequency() {
  return crystalFreq;
}

int16_t AX5x43::waitForXtal() {
  uint32_t start = mod->micros();
  uint32_t timeout = 5000000;

  while(!mod->SPIgetRegValue(RADIOLIB_AX5X43_REG_XTAL_STATUS)) {
    mod->yield();

    if(mod->micros() - start > timeout) return RADIOLIB_ERR_RANGING_TIMEOUT;

    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    mod->delayMicroseconds(100);
  }

  return RADIOLIB_ERR_NONE;
}

int16_t AX5x43::pllRanging() {
  uint8_t pll_loop_bak;
  uint8_t pll_cpi_bak;
  uint8_t pll_range = 0;

  // Set to standby (enabled ref & xtal if set)
  setMode(RADIOLIB_AX5X43_PWR_MODE_STANDBY);

  waitForXtal();

  pll_range = 0x18;
  
  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_PLL_RANGING_A, RADIOLIB_AX5X43_PLL_RNG_RST | 
                                                         RADIOLIB_AX5X43_PLL_RNG_START);

  while(pll_range & RADIOLIB_AX5X43_PLL_RNG_START) {
    pll_range = mod->SPIgetRegValue(RADIOLIB_AX5X43_REG_PLL_RANGING_A);
    RADIOLIB_DEBUG_PRINTLN(pll_range, HEX);
    delay(100);
  }

  RADIOLIB_DEBUG_PRINTLN(F("PLL ranging done"));

  if (pll_range & 0x20) return RADIOLIB_ERR_RANGING_TIMEOUT;

  return RADIOLIB_ERR_NONE;
}

int16_t AX5x43::setFrequency(float frq, bool forceRanging) {
  // check valid range
  RADIOLIB_CHECK_RANGE(frq, 27.0, 1050.0, RADIOLIB_ERR_INVALID_FREQUENCY);

  // calculate raw value
  uint32_t freqRaw = ((frq / getCrystalFrequency()) * (uint32_t(1) << RADIOLIB_AX5X43_DIV_EXPONENT));

  // force the LSB as per datasheet recommendation
  // at most, this will introduce 1.55 Hz error
  freqRaw |= 0x01;

  // set the registers (only frequency A is used right now)
  int16_t state = mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FREQ_A_3, (uint8_t)(freqRaw >> 24));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FREQ_A_2, (uint8_t)(freqRaw >> 16));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FREQ_A_1, (uint8_t)(freqRaw >> 8));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FREQ_A_0, (uint8_t)freqRaw);
  RADIOLIB_ASSERT(state);

  // check if we need to perform VCO autoranging
  if(forceRanging || (fabs(freq - frq) > 2.5f)) {
    // do ranging now
    state = pllRanging();

    // all done!
    state = standby();
    RADIOLIB_ASSERT(state);
  }

  // update cached frequency
  freq = frq;
  return(state);
}

int16_t AX5x43::setBitRate(float br) {
  // check valid range
  RADIOLIB_CHECK_RANGE(br, 0.1, 125.0, RADIOLIB_ERR_INVALID_BIT_RATE);

  // calculate raw value
  // TXRATE = [ ( BITRATE / f_XTAL ) * 2^24 + 1/2 ]
  // Bitrate is in K where f_XTAL is in M
  uint32_t brRaw = (br * (uint32_t(1) << RADIOLIB_AX5X43_DIV_EXPONENT)) / getCrystalFrequency() / 1000.0;

  // set the registers
  int16_t state = mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_RATE_2, (uint8_t)(brRaw >> 16));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_RATE_1, (uint8_t)(brRaw >> 8));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_TX_RATE_0, (uint8_t)brRaw);
  RADIOLIB_ASSERT(state);

  // update cached value
  bitRate = br;
  return(state);
}

int16_t AX5x43::setFrequencyDeviation(float freqDev) {
  // check valid range
  RADIOLIB_CHECK_RANGE(freqDev, 0, 125.0, RADIOLIB_ERR_INVALID_BIT_RATE);

  // calculate raw value
  // For FSK
  // FSKDEV = [ ( f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
  // For AFSK
  // FSKDEV = [ ( 0.858785 * f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
  uint32_t freqDevRaw = (freqDev * (uint32_t(1) << RADIOLIB_AX5X43_DIV_EXPONENT)) / getCrystalFrequency() / 1000;

  // set the registers
  int16_t state = mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FSK_DEV_2, (uint8_t)(freqDevRaw >> 16));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FSK_DEV_1, (uint8_t)(freqDevRaw >> 8));
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FSK_DEV_0, (uint8_t)freqDevRaw);
  return(state);
}

int16_t AX5x43::setPreambleLength(uint16_t preambleLength) {
  // check valid range
  RADIOLIB_CHECK_RANGE(preambleLength, 0, 32, RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH);

  // just update a cached variable - on AX5x43, preamble is actually written into FIFO
  preambleLen = preambleLength;
  return(RADIOLIB_ERR_NONE);
}

int16_t AX5x43::transmit(uint8_t* data, size_t len, uint8_t addr) {
  // calculate timeout (5ms + 500 % of expected time-on-air)
  uint32_t timeout = 5000000 + (uint32_t)((((float)(len * 8)) / (bitRate * 1000.0)) * 5000000.0);

  // start transmission
  int16_t state = startTransmit(data, len, addr);
  RADIOLIB_ASSERT(state);

  // TODO: Instead of delay implement device IRQ reading if not wired
  delay(1000);

  uint32_t start = mod->micros();
  while(!mod->digitalRead(mod->getIrq())) {
    mod->yield();

    if(mod->micros() - start > timeout) {
      finishTransmit();
      return(RADIOLIB_ERR_TX_TIMEOUT);
    }
  }

  return(finishTransmit());
}

int16_t AX5x43::receive(uint8_t* data, size_t len) {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

int16_t AX5x43::startTransmit(uint8_t* data, size_t len, uint8_t addr) {
  // clear fifo and set mode to full TX first
  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_FIFO_STAT, RADIOLIB_AX5X43_FIFO_CMD_CLEAR_ALL);
  int16_t state = setMode(RADIOLIB_AX5X43_PWR_MODE_FULL_TX);
  RADIOLIB_ASSERT(state);

  // write the preamble
  uint8_t pre[32 + 1];
  pre[0] = RADIOLIB_AX5X43_FIFO_TX_DATA_UNENC | RADIOLIB_AX5X43_FIFO_DATA_PKTSTART;
  memset(&pre[1], 0xAA, preambleLen);
  writeFifoChunk(RADIOLIB_AX5X43_FIFO_CHUNK_HDR_DATA, pre, preambleLen + 1);

  // write the data
  data[0] = RADIOLIB_AX5X43_FIFO_TX_DATA_UNENC | RADIOLIB_AX5X43_FIFO_DATA_PKTEND; 
  writeFifoChunk(RADIOLIB_AX5X43_FIFO_CHUNK_HDR_DATA, data, len);

  // wait until crystal is running
  state = waitForXtal();

  if(state != RADIOLIB_ERR_NONE) {
    finishTransmit();
    return(state);
  }

  // commit FIFO - this tarts the actual transmission
  mod->SPIwriteRegister(RADIOLIB_AX5X43_REG_FIFO_STAT, RADIOLIB_AX5X43_FIFO_CMD_COMMIT);
  return(RADIOLIB_ERR_NONE);
}

int16_t AX5x43::finishTransmit() {
  // set mode to standby to disable transmitter/RF switch
  return(standby());
}

int16_t AX5x43::readData(uint8_t* data, size_t len) {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

int16_t AX5x43::transmitDirect(uint32_t frf) {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

int16_t AX5x43::receiveDirect() {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

int16_t AX5x43::setDataShaping(uint8_t sh) {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

int16_t AX5x43::setEncoding(uint8_t encoding) {
  return(RADIOLIB_ERR_UNSUPPORTED);
}

size_t AX5x43::getPacketLength(bool update) {
  return(0);
}

uint8_t AX5x43::randomByte() {
  return(0);
}

void AX5x43::setDirectAction(void (*func)(void)) {

}

void AX5x43::readBit(RADIOLIB_PIN_TYPE pin) {

}

int16_t AX5x43::getChipVersion() {
  return(mod->SPIgetRegValue(RADIOLIB_AX5X43_REG_REVISION));
}

int16_t AX5x43::config() {
  // set the "performance tuning" magic registers
  int16_t state = mod->SPIsetRegValue(0xF00, 0x0F);
  state |= mod->SPIsetRegValue(0xF0D, 0x03);
  state |= mod->SPIsetRegValue(0xF1C, 0x07);
  state |= mod->SPIsetRegValue(0xF1C, 0x07);
  state |= mod->SPIsetRegValue(0xF44, 0x24);
  RADIOLIB_ASSERT(state);

  // From AND9347-D.PDF:
  // Internal Loop Filter x2, BW = 200 kHz for ICP = 272 mA
  // Bypass external filter
  state = mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_PLL_LOOP, 0x0A);
  // Charge pump current set to 16*8.5uA
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_PLL_CPI, 0x10);

  // TODO: Support dividing these for lower frequencies
  // Essenitally, try to range. If ranging fails, double the 
  // frequency and set the RFDIV bits here. Then retry ranging
  state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_PLL_VCO_DIV, 0x30);

  /// \todo change this based on TCXO presence
  state |= mod->SPIsetRegValue(0xF10, 0x04);
  state |= mod->SPIsetRegValue(0xF11, 0x00);
  RADIOLIB_ASSERT(state);

  /// \todo change this based on PLL RF divide-by-2
  state = mod->SPIsetRegValue(0xF34, 0x08);
  RADIOLIB_ASSERT(state);

  if(getCrystalFrequency() < 24.8f) {
    state = mod->SPIsetRegValue(0xF35, 0x10);
  } else {
    state = mod->SPIsetRegValue(0xF35, 0x11);
  }
  RADIOLIB_ASSERT(state);

  return(state);
}

int16_t AX5x43::setModulation(uint16_t modulation) {
  int16_t state = RADIOLIB_ERR_NONE;

  if(modulation == RADIOLIB_AX5X43_MODULATION_AFSK) {
    state |= mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_MODULATION, RADIOLIB_AX5X43_MODULATION_AFSK);

    // Mark and space at 2200Hz and 1200Hz
    state |=  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_AFSK_MARK_0,     0x14&0xFF);
    state |=  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_AFSK_MARK_1,  (0x14>>8)&0xFF);
    state |=  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_AFSK_SPACE_0,    0x24);
    state |=  mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_AFSK_SPACE_1, (0x24>>8)&0xFF);
  } else {
    state = RADIOLIB_ERR_UNSUPPORTED;
  }

  return(state);
}

int16_t AX5x43::setMode(uint8_t mode) {
  // Ignore WDS
  return(mod->SPIsetRegValue(RADIOLIB_AX5X43_REG_PWR_MODE, mode | crystalEnabled | refEnabled, 7, 0, 4, 0xEF));
}

void AX5x43::writeFifoChunk(uint8_t hdr, uint8_t* data, size_t len) {
  // write the header
  mod->SPIwriteRegister(RADIOLIB_AX5X43_REG_FIFO_DATA, hdr);
  if((data == NULL) || (len == 0)) {
    return;
  }

  // optionally, write the data
  // if it is one of the variable length chunks, write the length byte
  if((hdr == RADIOLIB_AX5X43_FIFO_CHUNK_HDR_DATA) || (hdr == RADIOLIB_AX5X43_FIFO_CHUNK_HDR_TXPWR)) {
    mod->SPIwriteRegister(RADIOLIB_AX5X43_REG_FIFO_DATA, len);
  }

  // now write the data
  for(size_t i = 0; i < len; i++) {
    mod->SPIwriteRegister(RADIOLIB_AX5X43_REG_FIFO_DATA, data[i]);
  }
}

#endif

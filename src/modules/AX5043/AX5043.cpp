#include "AX5043.h"
#if !defined(RADIOLIB_EXCLUDE_AX5043)

AX5043::AX5043(Module* mod) {
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

  if(!getChipRevision()) {
    RADIOLIB_DEBUG_PRINTLN(F("No AX5043 found!"));
    _mod->term();
    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  }

  int16_t state = configModulation(RADIOLIB_AX5043_MODULATION_AFSK);
  RADIOLIB_ASSERT(state);

  // Set to 3kHz deviation
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV, RADIOLIB_AX5043_FSKDEV_AFSK0);
  RADIOLIB_ASSERT(state);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+1, RADIOLIB_AX5043_FSKDEV_AFSK1);
  RADIOLIB_ASSERT(state);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FSKDEV+2, RADIOLIB_AX5043_FSKDEV_AFSK2);
  RADIOLIB_ASSERT(state);
  
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK, RADIOLIB_AX5043_AFSKMARK1);
  RADIOLIB_ASSERT(state);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKMARK+1, RADIOLIB_AX5043_AFSKMARK1);
  RADIOLIB_ASSERT(state);

  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE, RADIOLIB_AX5043_AFSKSPACE1);
  RADIOLIB_ASSERT(state);
  state =  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_AFSKSPACE+1, RADIOLIB_AX5043_AFSKSPACE1);
  RADIOLIB_ASSERT(state);

  freq = freq / 16e6;
  freq = freq * 16777216.0;
  freq = freq + 0.5;
  uint32_t fr = (uint32_t)freq;
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA,   fr);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-1, fr >> 8);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-2, fr >> 16);
  _mod->SPIsetRegValue(RADIOLIB_AX5043_REG_FREQA-3, fr >> 24);

  /*
    "begin" method SHOULD implement some sort of mechanism to verify the connection between Arduino and the module.

    For example, reading a version register
  */
}
#endif

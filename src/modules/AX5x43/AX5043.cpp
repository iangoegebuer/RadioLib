#include "AX5043.h"
#if !defined(RADIOLIB_EXCLUDE_AX5X43)

AX5043::AX5043(Module* mod) : AX5x43(mod) {

}

int16_t AX5043::begin(float freq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength) {
  AX5x43::crystalFreq = 16.368;
  int16_t state = AX5x43::begin(freq, br, freqDev, rxBw, power, preambleLength);
  return(state);
}

int16_t AX5043::beginAFSK(float freq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength) {
  AX5x43::crystalFreq = 16.368;
  AX5x43::refEnabled     = 1;
  AX5x43::divEnabled     = 1;
  AX5x43::crystalEnabled = 1;
  int16_t state = AX5x43::begin(freq, br, freqDev * 0.858785, rxBw, power, preambleLength);
  state |= AX5x43::setModulation(RADIOLIB_AX5X43_MODULATION_AFSK);

  return(state);
}

#endif

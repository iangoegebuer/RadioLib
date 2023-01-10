#if !defined(_RADIOLIB_AX5043_H)
#define _RADIOLIB_AX5043_H

#include "../../TypeDef.h"

#if !defined(RADIOLIB_EXCLUDE_AX5X43)

#include "../../Module.h"
#include "AX5x43.h"

/*!
  \class AX5043

  \brief Derived class for %AX5043 modules.
*/
class AX5043: public AX5x43 {
  public:

    // constructor

    /*!
      \brief Default constructor. Called from application layer when creating new module instance.

      \param mod Instance of Module that will be used to communicate with the transceiver chip.
    */
    AX5043(Module* mod);

    // basic methods

    int16_t begin(float freq = 434.0, float br = 4.8, float freqDev = 5.0, float rxBw = 125.0, int8_t power = 10, uint16_t preambleLength = 16);
    int16_t beginAFSK(float freq = 434.0, float br = 1.2, float freqDev = 3.0, float rxBw = 125.0, int8_t power = 10, uint16_t preambleLength = 16);
};

#endif

#endif

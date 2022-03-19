#include "../../protocols/PhysicalLayer/PhysicalLayer.h"
/*
  RadioLib Module Template header file

  Before opening pull request, please make sure that:
  1. All files MUST be compiled without errors using default Arduino IDE settings.
  2. All files SHOULD be compiled without warnings with compiler warnings set to "All".
  3. Example sketches MUST be working correctly and MUST be stable enough to run for prolonged periods of time.
  4. Writing style SHOULD be consistent.
  5. Comments SHOULD be in place for the most important chunks of code and SHOULD be free of typos.
  6. To indent, 2 spaces MUST be used.

  If at any point you are unsure about the required style, please refer to the rest of the modules.
*/

#if !defined(_RADIOLIB_AX5043_H) && !defined(RADIOLIB_EXCLUDE_AX5043)
#if !defined(_RADIOLIB_AX5043_H)
#define _RADIOLIB_AX5043_H

/*
  Header file for each module MUST include Module.h and TypeDef.h in the src folder.
  The header file MAY include additional header files.
*/
#include "../../Module.h"
#include "../../TypeDef.h"

/*
  Only use the following include if the module implements methods for OSI physical layer control.
  This concerns only modules similar to SX127x/RF69/CC1101 etc.

  In this case, your class MUST implement all virtual methods of PhysicalLayer class.
*/
//#include "../../protocols/PhysicalLayer/PhysicalLayer.h"

#define RADIOLIB_AX5043_FREQUENCY_STEP_SIZE                    16777216.0/16e6
#define RADIOLIB_AX5043_MAX_PACKET_LENGTH                      255

/*
  Register map
  Definition of SPI register map SHOULD be placed here. The register map SHOULD have two parts:

  1 - Address map: only defines register names and addresses. Register names MUST match names in
      official documentation (datasheets etc.).
  2 - Variable map: defines variables inside register. This functions as a bit range map for a specific register.
      Bit range (MSB and LSB) as well as short description for each variable MUST be provided in a comment.

  See RF69 and SX127x header files for examples of register maps.
*/
// AX5043 register map                             
#define RADIOLIB_AX5043_REG_SILICON_REVISION                   0x000
#define RADIOLIB_AX5043_REG_PWRMODE                            0x002
#define RADIOLIB_AX5043_REG_MODULATION                         0x010
#define RADIOLIB_AX5043_REG_TXRATE                             0x165
#define RADIOLIB_AX5043_REG_FSKDEV                             0x161
#define RADIOLIB_AX5043_REG_AFSKMARK                           0x112
#define RADIOLIB_AX5043_REG_AFSKSPACE                          0x110
#define RADIOLIB_AX5043_REG_FREQA                              0x037
#define RADIOLIB_AX5043_REG_TXPWRCOEFFB                        0x16B

#define RADIOLIB_AX5043_REG_PLLLOOP                            0x030
#define RADIOLIB_AX5043_REG_PLLCPI                             0x031
#define RADIOLIB_AX5043_REG_PLLRANGEA                          0x033
#define RADIOLIB_AX5043_REG_PLLVCODIV                          0x032

// RADIOLIB_AX5043_REG_MODULATION                                              MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_MODULATION_AFSK                        0b00001010  //  7     0     <description>
#define RADIOLIB_AX5043_MODULATION_FSK                         0b00001000  //  7     0     <description>

// RADIOLIB_AX5043_REG_TXRATE                                                  MSB   LSB   DESCRIPTION
// TXRATE = [ ( BITRATE / f_XTAL ) * 2^24 + 1/2 ]
// TXRATE is split into 3 registers starting at 0x167(LSB) and ending at 0x165(MSB)

// RADIOLIB_AX5043_REG_FSKDEV                                                  MSB   LSB   DESCRIPTION
// f_DEVIATION = Space/Mark deviation from center
// For FSK
// FSKDEV = [ ( f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
// For AFSK
// FSKDEV = [ ( 0.858785 * f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
#define RADIOLIB_AX5043_FSKDEV_AFSK2                             0x00      //  TYpical deviation of 3khz
#define RADIOLIB_AX5043_FSKDEV_AFSK1                             0x0A      //  TYpical deviation of 3khz
#define RADIOLIB_AX5043_FSKDEV_AFSK0                             0x8E      //  TYpical deviation of 3khz

// RADIOLIB_AX5043_REG_MODULATION                                              MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_AFSKMARK0                              0x00  
#define RADIOLIB_AX5043_AFSKMARK1                              0x14
#define RADIOLIB_AX5043_AFSKSPACE0                             0x00  
#define RADIOLIB_AX5043_AFSKSPACE1                             0x25  


// TXRATE is split into 3 registers starting at 0x167(LSB) and ending at 0x165(MSB)


/*
  Module class definition

  The module class MAY inherit from the following classes:

  1 - PhysicalLayer: In case the module implements methods for OSI physical layer control (e.g. SX127x).
  2 - Common class: In case the module further specifies some more generic class (e.g. SX127x/SX1278)
*/
class AX5043 : public PhysicalLayer {
  public:
    // introduce PhysicalLayer overloads
    using PhysicalLayer::transmit;
    using PhysicalLayer::receive;
    using PhysicalLayer::startTransmit;
    using PhysicalLayer::readData;

    /*
      Constructor MUST have only one parameter "Module* mod".
      The class MAY implement additional overloaded constructors.
    */
    // constructor
    AX5043(Module* mod);
    Module* getMod();

    /*
      The class MUST implement at least one basic method called "begin".
      The "begin" method MUST initialize the module and return the status as int16_t type.
    */
    // basic methods
    int16_t begin();
    int16_t beginAFSK(float freq = 144.390e6);
    int16_t getChipRevision();
    
/*!
      \brief Binary transmit method. Will transmit arbitrary binary data up to 255 bytes long using %LoRa or up to 63 bytes using FSK modem.
      For overloads to transmit Arduino String or C-string, see PhysicalLayer::transmit.
      \param data Binary data that will be transmitted.
      \param len Length of binary data to transmit (in bytes).
      \param addr Node address to transmit the packet to. Only used in FSK mode.
      \returns \ref status_codes
    */
    int16_t transmit(uint8_t* data, size_t len, uint8_t addr = 0) override;

    /*!
      \brief Binary receive method. Will attempt to receive arbitrary binary data up to 255 bytes long using %LoRa or up to 63 bytes using FSK modem.
      For overloads to receive Arduino String, see PhysicalLayer::receive.
      \param data Pointer to array to save the received binary data.
      \param len Number of bytes that will be received. Must be known in advance for binary transmissions.
      \returns \ref status_codes
    */
    int16_t receive(uint8_t* data, size_t len) override;

    /*!
      \brief Sets the %LoRa module to standby.
      \returns \ref status_codes
    */
    int16_t standby() override;

    /*!
      \brief Enables direct transmission mode on pins DIO1 (clock) and DIO2 (data).
      While in direct mode, the module will not be able to transmit or receive packets. Can only be activated in FSK mode.
      \param frf 24-bit raw frequency value to start transmitting at. Required for quick frequency shifts in RTTY.
      \returns \ref status_codes
    */
    int16_t transmitDirect(uint32_t frf = 0) override;

    /*!
      \brief Enables direct reception mode on pins DIO1 (clock) and DIO2 (data).
      While in direct mode, the module will not be able to transmit or receive packets. Can only be activated in FSK mode.
      \returns \ref status_codes
    */
    int16_t receiveDirect() override;

    /*!
      \brief Interrupt-driven binary transmit method. Will start transmitting arbitrary binary data up to 255 bytes long using %LoRa or up to 63 bytes using FSK modem.
      \param data Binary data that will be transmitted.
      \param len Length of binary data to transmit (in bytes).
      \param addr Node address to transmit the packet to. Only used in FSK mode.
      \returns \ref status_codes
    */
    int16_t startTransmit(uint8_t* data, size_t len, uint8_t addr = 0) override;

    /*!
      \brief Reads data that was received after calling startReceive method. This method reads len characters.
      \param data Pointer to array to save the received binary data.
      \param len Number of bytes that will be read. When set to 0, the packet length will be retreived automatically.
      When more bytes than received are requested, only the number of bytes requested will be returned.
      \returns \ref status_codes
    */
    int16_t readData(uint8_t* data, size_t len) override;

    /*!
      \brief Sets FSK frequency deviation from carrier frequency. Allowed values depend on bit rate setting and must be lower than 200 kHz. Only available in FSK mode.
      \param freqDev Frequency deviation to be set (in kHz).
      \returns \ref status_codes
    */
    int16_t setFrequencyDeviation(float freqDev) override;

    /*!
      \brief Query modem for the packet length of received payload.
      \param update Update received packet length. Will return cached value when set to false.
      \returns Length of last received packet in bytes.
    */
    size_t getPacketLength(bool update = true) override;

    /*!
      \brief Sets transmission encoding. Only available in FSK mode.
       Allowed values are RADIOLIB_ENCODING_NRZ, RADIOLIB_ENCODING_MANCHESTER and RADIOLIB_ENCODING_WHITENING.
      \param encoding Encoding to be used.
      \returns \ref status_codes
    */
    int16_t setEncoding(uint8_t encoding) override;

    /*!
      \brief Sets Gaussian filter bandwidth-time product that will be used for data shaping. Only available in FSK mode with FSK modulation.
      Allowed values are RADIOLIB_SHAPING_0_3, RADIOLIB_SHAPING_0_5 or RADIOLIB_SHAPING_1_0. Set to RADIOLIB_SHAPING_NONE to disable data shaping.
      \param sh Gaussian shaping bandwidth-time product that will be used for data shaping
      \returns \ref status_codes
    */
    int16_t setDataShaping(uint8_t sh) override;

    /*!
      \brief Function to read and process data bit in direct reception mode.
      \param pin Pin on which to read.
    */
    void readBit(RADIOLIB_PIN_TYPE pin);

    /*!
      \brief Set interrupt service routine function to call when data bit is receveid in direct mode.
      \param func Pointer to interrupt service routine.
    */
    void setDirectAction(void (*func)(void));

    /*!
     \brief Get one truly random byte from RSSI noise.
     \returns TRNG byte.
   */
    uint8_t randomByte();

#if !defined(RADIOLIB_GODMODE)
  private:
#endif
    /*
      The class MUST contain private member "Module* _mod"
    */
    Module* _mod;

    /*
      The class MAY contain additional private variables and/or methods.
      Private member variables MUST have a name prefixed with "_" (underscore, ASCII 0x5F)

      Usually, these are variables for saving module configuration, or methods that do not have to be exposed to the end user.
    */
    int16_t configModulation(uint8_t modulation);
    int16_t pllRanging();
};

#endif

#endif

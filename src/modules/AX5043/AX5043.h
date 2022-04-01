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
#define RADIOLIB_AX5043_REG_IRQMASK0                           0x007
#define RADIOLIB_AX5043_REG_RADIOEVENTMASK0                    0x009
#define RADIOLIB_AX5043_REG_RADIOEVENTREQ0                     0x00F
#define RADIOLIB_AX5043_REG_MODULATION                         0x010
#define RADIOLIB_AX5043_REG_ENCODING                           0x011
#define RADIOLIB_AX5043_REG_TXRATE                             0x167 // 0x167(LSB) - 0x165(MSB)
#define RADIOLIB_AX5043_REG_FSKDEV                             0x161
#define RADIOLIB_AX5043_REG_AFSKMARK                           0x112
#define RADIOLIB_AX5043_REG_AFSKSPACE                          0x110
#define RADIOLIB_AX5043_REG_FREQA                              0x037 // 0x037(LSB) - 0x034(MSB)
#define RADIOLIB_AX5043_REG_TXPWRCOEFFB                        0x16B

#define RADIOLIB_AX5043_REG_PLLLOOP                            0x030
#define RADIOLIB_AX5043_REG_PLLCPI                             0x031
#define RADIOLIB_AX5043_REG_PLLRANGEA                          0x033
#define RADIOLIB_AX5043_REG_PLLVCODIV                          0x032

// FIFO registers
#define RADIOLIB_AX5043_REG_FIFOSTAT                           0x028
#define RADIOLIB_AX5043_REG_FIFODATA                           0x029

// Unnamed registers
#define RADIOLIB_AX5043_REG_F34                                0xF34
#define RADIOLIB_AX5043_REG_F35                                0xF35
#define RADIOLIB_AX5043_REG_F10                                0xF10
#define RADIOLIB_AX5043_REG_F11                                0xF11

// RADIOLIB_AX5043_REG_PWRMODE                                                 MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_PWRMODE_FULL_TX                        0x0D  //  7     0     <description>
#define RADIOLIB_AX5043_PWRMODE_POWERDOWN                      0x00  //  7     0     <description>

// RADIOLIB_AX5043_REG_MODULATION                                              MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_MODULATION_AFSK                        0b00001010  //  7     0     <description>
#define RADIOLIB_AX5043_MODULATION_FSK                         0b00001000  //  7     0     <description>

// RADIOLIB_AX5043_REG_Encoding                                                MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_ENCODING_DIFF                          0b00000010  //  7     0     <description>
#define RADIOLIB_AX5043_ENCODING_INVERTED                      0b00000001  //  7     0     <description>
#define RADIOLIB_AX5043_ENCODING_NRZ                           ( RADIOLIB_AX5043_ENCODING_DIFF | RADIOLIB_AX5043_ENCODING_INVERTED )

// RADIOLIB_AX5043_REG_FIFOSTAT_CMD(AND9347-D.PDF Table 64)                    MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_FIFOSTAT_CMD_ASK_COHERENT              0b00000001  //  7     0     <description>
#define RADIOLIB_AX5043_FIFOSTAT_CMD_CLEAR_ERROR               0b00000010  //  7     0     <description>
#define RADIOLIB_AX5043_FIFOSTAT_CMD_CLEAR_FIFO                0b00000011  //  7     0     <description>
#define RADIOLIB_AX5043_FIFOSTAT_CMD_COMMIT                    0b00000100  //  7     0     <description>
#define RADIOLIB_AX5043_FIFOSTAT_CMD_ROLLBACK                  0b00000101  //  7     0     <description>

// RADIOLIB_AX5043_REG_FIFODATA_HDR (AND9347-D.PDF Table 3)                    MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_REG_FIFODATA_HDR_SINGLE                0b00100000  //  7     0     <description>
#define RADIOLIB_AX5043_REG_FIFODATA_HDR_DOUBLE                0b01000000  //  7     0     <description>
#define RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE               0b01100000  //  7     0     <description>
#define RADIOLIB_AX5043_REG_FIFODATA_HDR_VARIABLE              0b11100000  //  7     0     <description>

// RADIOLIB_AX5043_REG_FIFODATA_TYPE (AND9347-D.PDF Table 4)                   MSB   LSB   DESCRIPTION
// Single byte header 
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_RSSI                (0b00010001|RADIOLIB_AX5043_REG_FIFODATA_HDR_SINGLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_TXCTR               (0b00011100|RADIOLIB_AX5043_REG_FIFODATA_HDR_SINGLE)
// Double byte header 
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_FREQ_OFFSET         (0b00010010|RADIOLIB_AX5043_REG_FIFODATA_HDR_DOUBLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_ANTRSSI             (0b00010101|RADIOLIB_AX5043_REG_FIFODATA_HDR_DOUBLE)
// Tripple byte header 
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_REPEAT_DATA         (0b00000010|RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_TIMER               (0b00010000|RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_RF_FREQ_OFFSET      (0b00010011|RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_DATA_RATE           (0b00010100|RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_ANT_RSSI_SEL        (0b00010101|RADIOLIB_AX5043_REG_FIFODATA_HDR_TRIPPLE)
// Variable byte header 
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_DATA                (0b00000001|RADIOLIB_AX5043_REG_FIFODATA_HDR_VARIABLE)
#define RADIOLIB_AX5043_REG_FIFODATA_TYPE_TXPWR               (0b00011101|RADIOLIB_AX5043_REG_FIFODATA_HDR_VARIABLE)

// RADIOLIB_AX5043_REG_TXRATE                                                  MSB   LSB   DESCRIPTION
// TXRATE = [ ( BITRATE / f_XTAL ) * 2^24 + 1/2 ]
// TXRATE is split into 3 registers starting at 0x167(LSB) and ending at 0x165(MSB)

// RADIOLIB_AX5043_REG_FSKDEV                                                  MSB   LSB   DESCRIPTION
// f_DEVIATION = Space/Mark deviation from center
// For FSK
// FSKDEV = [ ( f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
// For AFSK
// FSKDEV = [ ( 0.858785 * f_DEVIATION / f_XTAL ) * 2^24 + 1/2 ]
#define RADIOLIB_AX5043_FSKDEV_AFSK2                           0x00      //  TYpical deviation of 3khz
#define RADIOLIB_AX5043_FSKDEV_AFSK1                           0x0A      //  TYpical deviation of 3khz
#define RADIOLIB_AX5043_FSKDEV_AFSK0                           0x8E      //  TYpical deviation of 3khz

// RADIOLIB_AX5043_REG_MODULATION                                              MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_AFSKMARK1                              0x00  
#define RADIOLIB_AX5043_AFSKMARK0                              0x14

#define RADIOLIB_AX5043_AFSKSPACE1                             0x00  
#define RADIOLIB_AX5043_AFSKSPACE0                             0x24  

// RADIOLIB_AX5043_REG_PLLRANGEA                                              MSB   LSB   DESCRIPTION
#define RADIOLIB_AX5043_REG_PLLRANGEA_VCORANGE                 0b00001111  //  3     0    ? How much deviation after lock ?
#define RADIOLIB_AX5043_REG_PLLRANGEA_VCORANGE_RST             0b00001000  //  3     0    Reset value of VCO range
#define RADIOLIB_AX5043_REG_PLLRANGEA_RNG_START                0b00010000  //  4     4    PLL Ranging start. Cleared when complete
#define RADIOLIB_AX5043_REG_PLLRANGEA_RNG_ERROR                0b00100000  //  5     5    PLL Ranging error/failed (check frequncy/L_ext)
#define RADIOLIB_AX5043_REG_PLLRANGEA_PLL_LOCK                 0b01000000  //  6     6    PLL is locked
#define RADIOLIB_AX5043_REG_PLLRANGEA_STICK_LOCK               0b10000000  //  7     7    PLL lost lock after last read if 0

// RADIOLIB_AX5043_REG_F34
#define RADIOLIB_AX5043_F34_NO_RFDIV                           0x08  //  RFDIV is not used
#define RADIOLIB_AX5043_F34_RFDIV                              0x28  //  RFDIV used

// RADIOLIB_AX5043_REG_F35
// 0x10 XTAL/TCXO < 24.8 MHz, (fXTALDIV = 1)
// 0x11 otherwise             (fXTALDIV = 2)
#define RADIOLIB_AX5043_F35_XTALDIV_1                          0x10  //  RFDIV is not used
#define RADIOLIB_AX5043_F35_XTALDIV_2                          0x11  //  RFDIV used

// RADIOLIB_AX5043_REG_F10
// Set to 0x04 if a TCXO is used. 
// If a crystal(>43MHz) is used set to 0x0D
// Set to 0x03 otherwise
#define RADIOLIB_AX5043_F10_TCXO                               0x04  // TCXO is used
#define RADIOLIB_AX5043_F10_XTAL_HF                            0x0D  // TXCO + XTAL > 43MHz
#define RADIOLIB_AX5043_F10_XTAL_LF                            0x03  // "Otherwise"

// RADIOLIB_AX5043_REG_F11
// Set to 0x07 if a XTAL 
// Set to 0x00 if a TCXO
#define RADIOLIB_AX5043_F11_TCXO                               0x00  // TCXO is used
#define RADIOLIB_AX5043_F11_XTAL                               0x07  // XTAL is used


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

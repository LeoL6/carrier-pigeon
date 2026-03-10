#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "LoRa.h"

namespace LoRa
{
  // <=================================>
  //   LoRa config for North America
  // <=================================>
  static constexpr int RF_FREQUENCY                = 915E6; //Hz
  static constexpr int TX_OUTPUT_POWER             = 14;    //dbM
  static constexpr int LORA_BANDWIDTH              = 0;     // 0:125kHz, 1:250kHz, 2:500kHz
  static constexpr int LORA_SPREADING_FACTOR       = 7;    
  static constexpr int LORA_CODINGRATE             = 1;     //4/5
  static constexpr int LORA_PREAMBLE_LENGTH        = 8;     
  static constexpr bool LORA_FIX_LENGTH_PAYLOAD_ON = false; 
  static constexpr bool LORA_IQ_INVERSION_ON       = false; 

  // <=========================>
  //   Radio event structure
  // <=========================>
  RadioEvents_t RadioEvents;

  // <=========================>
  //   TX Variables
  // <=========================>
  static uint8_t txBuffer[LoRa::BUFFER_SIZE];
  static volatile bool txInProgress = false;

  // <=========================>
  //   RX Variables
  // <=========================>
  static uint8_t rxBuffer[LoRa::BUFFER_SIZE];
  static uint16_t rxSize = 0;
  static volatile bool messageAvailable = false;

  // struct Message
  // {
  //     uint8_t data[256];
  //     uint16_t size;
  //     int16_t rssi;
  //     int8_t snr;
  // };

  // <=========================>
  //   Forward declarations
  // <=========================>
  static void sendLoRaMessage(const String &msg);
  static void onTxDone();
  static void onTxTimeout(void);
  static void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
  static void onRxTimeout(void);
  static void onRxError(void);

  // Setup function
  void init() 
  {
    Serial.println("Initializing Heltec LoRa...");
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    // Attach event handlers
    RadioEvents.TxDone = onTxDone;
    RadioEvents.TxTimeout = onTxTimeout;
    RadioEvents.RxDone = onRxDone;
    RadioEvents.RxTimeout = onRxTimeout;
    RadioEvents.RxError = onRxError;

    // Init radio
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);

    // Configure transmission
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
                    
    // Configure reception
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    0, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    Serial.println("LoRa initialized (Heltec native driver)");
    Radio.Rx(0);  // Start listening immediately
  }

  // Send LoRa message
  void sendMessage(const char* msg)
  {
    if (txInProgress) return;

    size_t size = strlen(msg);
    if (size > BUFFER_SIZE)
        size = BUFFER_SIZE;

    memcpy(txBuffer, msg, size);

    txInProgress = true;

    Serial.println("Sending...");
    Radio.Send(txBuffer, size);
  }

  void onTxDone() 
  {
    txInProgress = false;

    // Switch back to receive mode
    Radio.Rx(0);
  }

  void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) 
  {
    if (size > BUFFER_SIZE)
      size = BUFFER_SIZE;

    memcpy(rxBuffer, payload, size);
    rxSize = size;
    messageAvailable = true;

    // Switch back to receive mode
    Radio.Rx(0);
  }

  void onTxTimeout(void)
  {
    txInProgress = false;

    // Switch back to receive mode
    Radio.Rx(0);
  }

  void onRxTimeout(void)
  {
    // Switch back to receive mode
    Radio.Rx(0);
  }

  void onRxError(void)
  {
    // Switch back to receive mode
    Radio.Rx(0);
  }

  void update() 
  {
    Radio.IrqProcess(); // Required occasionally for RX/TX Callbacks to be triggered
  }

  bool isMessageAvailable()
  {
      return messageAvailable;
  }

  uint16_t getMessage(uint8_t* buffer)
  {
      if (!messageAvailable || !buffer)
          return 0;

      memcpy(buffer, rxBuffer, rxSize);
      messageAvailable = false;

      return rxSize;
  }
}

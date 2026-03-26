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
  //   Radio Events
  // <=========================>
  RadioEvents_t RadioEvents;

  // <=========================>
  //   Callback Variables
  // <=========================>
  static ReceiveCallback onReceiveCb = nullptr;

  // <=========================>
  //   TX Variables
  // <=========================>
  static uint8_t txBuffer[LoRa::BUFFER_SIZE];
  static volatile bool txInProgress = false;

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

  void setReceiveCallback(ReceiveCallback cb)
  {
    onReceiveCb = cb;
  }

  void onTxDone() 
  {
    Serial.println("TRANSMIT DONE");
    txInProgress = false;

    // Switch back to receive mode
    Radio.Rx(0);
  }

  void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) 
  {
    Serial.println("PACKET RECEIEVED");

    if (onReceiveCb)
    {
      onReceiveCb(payload, size);
    }

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

  static bool isQueueFull()
  {
    return ((queueHead + 1) % QUEUE_SIZE) == queueTail;
  }

  static bool isQueueEmpty()
  {
    return queueHead == queueTail;
  }

  static void sendNow(const QueuedPacket& pkt)
  {
    txBuffer[0] = pkt.type;
    txBuffer[1] = pkt.length;

    if (pkt.length > 0)
        memcpy(&txBuffer[2], pkt.payload, pkt.length);

    size_t totalSize = pkt.length + 2;

    txInProgress = true;

    Serial.println("Sending packet...");
    Radio.Send(txBuffer, totalSize);
  }

  bool enqueuePacket(uint8_t type, const uint8_t* payload, uint8_t len)
  {
    if (len > BUFFER_SIZE)
      return false;

    if (isQueueFull())
    {
      Serial.println("QUEUE FULL");
      return false;
    }

    QueuedPacket& pkt = sendQueue[queueHead];

    pkt.type = type;
    pkt.length = len;

    if (len > 0 && payload)
      memcpy(pkt.payload, payload, len);

    queueHead = (queueHead + 1) % QUEUE_SIZE;

    return true;
  }

  void update() 
  {
    // Required occasionally for RX/TX Callbacks to be triggered
    Radio.IrqProcess();

    // Only send if radio is free
    if (txInProgress) return;

    if (isQueueEmpty()) return;

    // Dequeue
    QueuedPacket& pkt = sendQueue[queueTail];
    queueTail = (queueTail + 1) % QUEUE_SIZE;

    sendNow(pkt);
  }
}

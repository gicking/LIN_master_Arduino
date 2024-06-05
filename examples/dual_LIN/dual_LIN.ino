/**
  \file     dual_LIN.ino
  \example  dual_LIN.ino
  \brief    Dual LIN master node emulation
  \details  Simple emulation of two LIN master nodes via Serial1 (19.2kBaud) & Serial2 (9.6kBaud). Only master request frames are sent
  \author   Georg Icking-Konert

  \note 
  The sender state machine relies on reading back its 1-wire echo. 
  If no LIN or K-Line transceiver is used, connect Rx&Tx (only 1 device!) 
*/

// LIN master library
#include "LIN_master1.h"
#include "LIN_master2.h"

// task scheduler library. Either "Task Scheduler" library (AVR, SAM), or Ticker (ESP32, ESP8266)
#if defined(__AVR__) || defined(__SAM3X8E__)
  #include "Tasks.h"
#elif defined(ESP32) || defined(ESP8266)
  #include "Ticker.h" 
  Ticker  ticker_LIN1;
  Ticker  ticker_LIN2;
#else
  #error no supported task library found. Install either "Task Scheduler" library (AVR, SAM), or Ticker (ESP32, ESP8266)
#endif


// pin to demonstrate background operation
#define PIN_TOGGLE    30

// task scheduler periods [ms]
#define LIN1_PERIOD    20
#define LIN2_PERIOD    20


void setup(void)
{
  // show background operation
  pinMode(PIN_TOGGLE, OUTPUT);
  
  // for debug
  Serial.begin(115200); while(!Serial);

  // initialize LIN master (background operation)
  LIN_master0.begin(19200, LIN_V2, true);
  LIN_master1.begin(9600, LIN_V1, true);
  
  // init task scheduler (also required for LIN master emulation!)
  #if defined(__AVR__) || defined(__SAM3X8E__)
    Tasks_Init();
    Tasks_Add((Task) sendMasterRequest1, LIN1_PERIOD, 0);
    Tasks_Add((Task) sendMasterRequest2, LIN2_PERIOD, LIN1_PERIOD/2);
    Tasks_Start();
  #else // ESP32 / ESP8266
    ticker_LIN1.attach_ms(LIN1_PERIOD, sendMasterRequest1);
    delay(LIN1_PERIOD/2);
    ticker_LIN2.attach_ms(LIN2_PERIOD, sendMasterRequest2);
  #endif

} // setup()



void loop(void)
{
  // toggle pin to show background operation
  digitalWrite(PIN_TOGGLE, !digitalRead(PIN_TOGGLE));
  
} // loop()



// send master request via Serial1
void sendMasterRequest1(void)
{
  // assert that LIN state is idle
  if (LIN_master1.getState() != LIN_STATE_IDLE)
    return;
  
  // assemble frame data
  static uint8_t  count=0;
  uint8_t         id = 0x05;
  uint8_t         lenData = 8;
  uint8_t         data[8];
  memset(data, count++, lenData);
  
  // send master request
  LIN_master1.sendMasterRequest(id, lenData, data);

} // sendMasterRequest1()



// send master request via Serial2
void sendMasterRequest2(void)
{
  // assert that LIN state is idle
  if (LIN_master2.getState() != LIN_STATE_IDLE)
    return;
  
  // assemble frame data
  static uint8_t  count=127;
  uint8_t         id = 0x11;
  uint8_t         lenData = 8;
  uint8_t         data[8];
  memset(data, count++, lenData);
  
  // send master request
  LIN_master2.sendMasterRequest(id, lenData, data);

} // sendMasterRequest2()

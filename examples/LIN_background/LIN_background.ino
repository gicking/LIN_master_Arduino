/**
  \file     LIN_background.ino
  \example  LIN_background.ino
  \brief    Simple LIN master node emulation with background operation
  \details  Simple emulation of a LIN master node via Serial1 (+ LIN transceiver) for DAI MRA2 FED1.0 with background operation. Status is printed periodically.
  \author   Georg Icking-Konert

  \note 
  The sender state machine relies on reading back its 1-wire echo. 
  If no LIN or K-Line transceiver is used, connect Rx&Tx (only 1 device!) 
*/

// LIN master library
#include "LIN_master1.h"

// task scheduler library. Either "Task Scheduler" library (AVR, SAM), or Ticker (ESP32, ESP8266)
#if defined(__AVR__) || defined(__SAM3X8E__)
  #include "Tasks.h"
#elif defined(ESP32) || defined(ESP8266)
  #include "Ticker.h" 
  Ticker  ticker_LIN;
  Ticker  ticker_Print;
#else
  #error no supported task library found. Install either "Task Scheduler" library (AVR, SAM), or Ticker (ESP32, ESP8266)
#endif


// task scheduler periods [ms]
#define PRINT_PERIOD  1000    // period for status output
#define LIN_PERIOD    10      // LIN frame every N ms

// pin to demonstrate background operation
#define PIN_TOGGLE    30

// helper routine to print slave status
void printStatus(void);


// global variables
uint8_t  Rx[8];         // received data


void setup(void)
{
  // show background operation
  pinMode(PIN_TOGGLE, OUTPUT);
  
  // for user interaction via console
  Serial.begin(115200); while(!Serial);
  
  // initialize LIN master (background operation)
  LIN_master1.begin(19200, LIN_V2, true);
  
  // init task scheduler (also required for LIN master emulation!)
  #if defined(__AVR__) || defined(__SAM3X8E__)
    Tasks_Init();
    Tasks_Add((Task) LIN_scheduler, LIN_PERIOD, 0);
    Tasks_Add((Task) printStatus, PRINT_PERIOD, PRINT_PERIOD);
    Tasks_Start();
  #else // ESP32 / ESP8266
    ticker_LIN.attach_ms(LIN_PERIOD, LIN_scheduler);
    ticker_Print.attach_ms(PRINT_PERIOD, printStatus);
  #endif

} // setup()



void loop(void)
{
  // toggle pin to show background operation
  digitalWrite(PIN_TOGGLE, !digitalRead(PIN_TOGGLE));
  
} // loop()



// actual LIN scheduler. Periodically called by task scheduler
void LIN_scheduler(void)
{
  static uint8_t  count = 0;
  uint8_t         id;
  uint8_t         numData;
  uint8_t         data[8];

  // assert that LIN bus is idle
  if (LIN_master1.getState() != LIN_STATE_IDLE)
    return;

  // FED1.0 Daimler MRA2: speed request
  if (count == 0) {
  
    // assemble frame data (dummy)
    id      = 0x3B;
    numData = 2;
    memset(data, 0, 8);

    // send master request
    LIN_master1.sendMasterRequest(id, numData, data);

    // advance to next message
    count++;

  } // count == 0

  else {

    // assemble frame data
    id      = 0x1B;
    numData = 8;

    // get slave status. Copy data to buffer after reception
    LIN_master1.receiveSlaveResponse(id, numData, Rx);

    // restart LIN scheduler
    count=0;

  } // count == 1
  
} // LIN_scheduler()



// print slave response signals. Periodically called by task scheduler
void printStatus(void)
{
  // no LIN frame is ongoing -> print data
  if (LIN_master1.getState() == LIN_STATE_IDLE)
  {
    uint8_t rx[8];    // local buffer for global Rx data

    // copy data locally. Pause LIN scheduler temporarily for consistency
    #if defined(__AVR__) || defined(__SAM3X8E__)
      Tasks_Pause();
      memcpy(rx, Rx, 8);
      Tasks_Start();
    #else // ESP32 / ESP8266
      ticker_LIN.detach();
      memcpy(rx, Rx, 8);
      ticker_LIN.attach_ms(LIN_PERIOD, LIN_scheduler);
    #endif 
    
    // print time
    Serial.print(millis()); Serial.println("ms");
  
    // LIN ok -> print received data
    if (LIN_master1.error == LIN_SUCCESS)
    {
      for (uint8_t i=0; i<8; i++)
      {
        Serial.print(i); Serial.print("\t0x"); Serial.println(rx[i], HEX);
      }
    }
  
    // print LIN error status
    else {
      Serial.print("LIN error (0x");
      Serial.print(LIN_master1.error, HEX);
      Serial.print("): ");
      if (LIN_master1.error & LIN_ERROR_STATE)
        Serial.println("statemachine");
      else if (LIN_master1.error & LIN_ERROR_ECHO)
        Serial.println("echo");
      else if (LIN_master1.error & LIN_ERROR_TIMEOUT)
        Serial.println("timeout");
      else if (LIN_master1.error & LIN_ERROR_CHK)
        Serial.println("checksum");
      else if (LIN_master1.error & LIN_ERROR_MISC)
        Serial.println("misc");
    } // error

    Serial.println();

    // reset latched error and flag for data received
    LIN_master1.error = LIN_SUCCESS;

  } // LIN state == idle


  // LIN communication ongoing -> re-try in 2ms
  else
  {
    #if defined(__AVR__) || defined(__SAM3X8E__)
      Tasks_Delay(printStatus, 2);
    #else // ESP32 / ESP8266
      ticker_Print.detach();
      delay(2);
      printStatus();
      ticker_Print.attach_ms(PRINT_PERIOD, printStatus);
    #endif
  }
  
} // printStatus()

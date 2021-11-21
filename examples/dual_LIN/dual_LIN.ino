/**
  \file     dual_LIN.ino
  \example  dual_LIN.ino
  \brief    Dual LIN master node emulation
  \details  Simple emulation of two LIN master nodes via Serial1 (19.2kBaud) & Serial2 (9.6kBaud). Only master request frames are sent
  \author   Georg Icking-Konert
  \date     2020-03-15

  \note 
  The sender state machine relies on reading back its 1-wire echo. 
  If no LIN or K-Line transceiver is used, connect Rx&Tx (only 1 device!) 
*/

// include files
#include "LIN_master1.h"
#include "LIN_master2.h"
#include "Tasks.h"

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
  LIN_master1.begin(19200, LIN_V2, true);
  LIN_master2.begin(9600, LIN_V1, true);
  
  // init task scheduler (also required for LIN master emulation!)
  Tasks_Init();
  Tasks_Add((Task) sendMasterRequest1, LIN1_PERIOD, 0);
  Tasks_Add((Task) sendMasterRequest2, LIN2_PERIOD, LIN1_PERIOD/2);
  Tasks_Start();

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

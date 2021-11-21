/**
  \file     LIN_blocking.ino
  \example  LIN_blocking.ino
  \brief    Simple LIN master node emulation with blocking operation
  \details  Simple emulation of a LIN master node via Serial3 (+ LIN transceiver) for DAI MRA2 FED1.0 with blocking operation. Status is printed periodically.
  \author   Georg Icking-Konert
  \date     2020-03-15

  \note 
  The sender state machine relies on reading back its 1-wire echo. 
  If no LIN or K-Line transceiver is used, connect Rx&Tx (only 1 device!) 
*/

// include files                                                                       
#include "LIN_master3.h"      // muDuino LIN via Serial3

// pin to demonstrate background operation
#define PIN_TOGGLE    30

// pause between LIN frames
#define LIN_PAUSE     250


void setup(void)
{
  // show background operation
  pinMode(PIN_TOGGLE, OUTPUT);
  
  // for user interaction via console
  Serial.begin(115200); while(!Serial);
  
  // initialize LIN master (blocking operation)
  LIN_master3.begin(19200, LIN_V2, false);

} // setup()



void loop(void)
{
  static uint32_t lastCall = LIN_PAUSE;
  static uint8_t  count = 0;
  uint8_t         Tx[2] = {0,0};
  uint8_t         Rx[8];
  
  // toggle pin to show background operation
  digitalWrite(PIN_TOGGLE, !digitalRead(PIN_TOGGLE));

  // simple LIN scheduler
  if (millis() - lastCall > LIN_PAUSE) {
    lastCall = millis();

    // send master request
    if (count == 0) {
      count++;
  
      // dummy master request (blocking)
      LIN_master3.sendMasterRequest(0x3B, 2, Tx);
      
    } // count == 0
  
    // receive slave response
    else {
      count=0;
      
      // receive slave response (blocking)
      LIN_master3.receiveSlaveResponse(0x1B, 8, Rx);
  
      // print status and data
      Serial.print(millis()); Serial.println("ms");
      Serial.print("error: "); Serial.println(LIN_master3.error);
      Serial.println("data:");
      for (uint8_t i=0; i<8; i++)
      {
        Serial.print("  "); Serial.print(i); Serial.print("\t0x"); Serial.println(Rx[i], HEX);
      }
      Serial.println();
      
      // clear pending error and flag for received data
      LIN_master3.error = LIN_SUCCESS;
      
    } // count == 1

  } // scheduler
    
} // loop()

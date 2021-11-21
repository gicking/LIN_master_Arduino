/**
  \file     LIN_callback.ino
  \example  LIN_callback.ino
  \brief    Single LIN master node emulation for FED1.0 DAI MRA2.
  \details  Emulation of a LIN master node via Serial3 (+ LIN transceiver) with callback function for DAI MRA2 FED1.0. Speed can be controlled and status is printed.
  \author   Georg Icking-Konert
  \date     2020-03-15

  \note 
  The sender state machine relies on reading back its 1-wire echo. 
  If no LIN or K-Line transceiver is used, connect Rx&Tx (only 1 device!) 
*/

// include files
#include "LIN_master3.h"      // muDuino LIN via Serial3
#include "Tasks.h"

// task scheduler periods [ms]
#define PRINT_PERIOD  1000    // period for status output
#define LIN_PERIOD    10      // LIN frame every N ms

// pin to demonstrate background operation
#define PIN_TOGGLE    30

// helper routine to print slave status
void printStatus(void);


// FED1.0 Daimler MRA2 LIN signals
uint8_t  CoolFan_RPM = 0;                 // target speed request [%Nnom]
uint8_t  CoolFan_RPM_Ack = 0;             // target speed acknowledge [%Nnom]
uint8_t  CoolFan_EmAct_Stat = 0;          // status emergency mode
uint8_t  CoolFan_RPM_Avg = 0;             // actual speed [%Nnom]=[25rpm]
uint8_t  CoolFan_OvrVolt_Stat = 0;        // status overvoltage shutdown
uint8_t  CoolFan_Voltage_Avg = 0;         // supply voltage [0.2V]
uint8_t  CoolFan_UnderVolt_Stat = 0;      // status undervoltage shutdown
uint8_t  CoolFan_Current_Avg = 0;         // supply current [A]
uint8_t  CoolFan_VoltDerat_Stat = 0;      // status voltage derating
uint8_t  CoolFan_cur_Temp = 0;            // PCB temperature [C]
uint8_t  CoolFan_TempDerat_Stat = 0;      // status temperature derating
uint8_t  CoolFan_Stiff_Stat = 0;          // status sluggishness detection
uint8_t  CoolFan_Blocking_Stat = 0;       // status blocking detection
uint8_t  CoolFan_Electr_Err = 0;          // status internal error
uint8_t  CoolFan_Mech_Err = 0;            // status mechanical error
uint8_t  CoolFan_OvrTemp_Err = 0;         // status overtemperature
uint8_t  CoolFan_Err_Group_ERR_Stat = 0;  // collective error status 
uint8_t  CoolFan_Err_Group_SNA_Stat = 0;  // status Tpcb or ADC error
uint8_t  CoolFan_Type = 0;                // cooler fan type
uint8_t  RsErr_CF = 0;                    // ?
uint8_t  WakeupStat_CF = 0;               // ?



void setup(void)
{
  // show background operation
  pinMode(PIN_TOGGLE, OUTPUT);
  
  // for user interaction via console
  Serial.begin(115200); while(!Serial);
  
  // initialize LIN master (background operation)
  LIN_master3.begin(19200, LIN_V2, true);
  
  // init task scheduler (also required for LIN master emulation!)
  Tasks_Init();
  Tasks_Add((Task) LIN_scheduler, LIN_PERIOD, 0);
  Tasks_Add((Task) printStatus, PRINT_PERIOD, PRINT_PERIOD);
  Tasks_Start();

} // setup()



void loop(void)
{
  // toggle pin to show background operation
  digitalWrite(PIN_TOGGLE, !digitalRead(PIN_TOGGLE));
  
  // set new RPM via keys
  if (Serial.available())
  {
    uint8_t c = Serial.read();
    switch(c)
    {
      // 1 -> reduce speed by 10%
      case '1':
        CoolFan_RPM = (uint8_t) max(0, (int) CoolFan_RPM - 10);
        break;
        
      // 2 -> increase speed by 10%
      case '2':
        CoolFan_RPM = (uint8_t) min(127, (int) CoolFan_RPM + 10);
        break;
        
      // 0 -> motor off
      case '0':
        CoolFan_RPM = 0;
        break;
        
    } // switch key
    
    // print speed
    Serial.print("new speed = ");
    Serial.print(CoolFan_RPM*25);
    Serial.println("rpm");

  } // key pressed
  
} // loop()



// actual LIN scheduler. Periodically called by task scheduler
void LIN_scheduler(void)
{
  static uint8_t  count = 0;
  uint8_t         id;
  uint8_t         numData;
  uint8_t         data[8];

  // assert that LIN state is idle
  if (LIN_master3.getState() != LIN_STATE_IDLE)
    return;
  
  // debug
  //count = 0;
  
  // FED1.0 Daimler MRA2: speed request
  if (count == 0) {
  
    // assemble frame data
    id      = 0x3B;
    numData = 2;
    memset(data, 0, 8);
    data[0] = CoolFan_RPM & 0x7f;   // set speed from global variable. Clear bit 7 ("Notlaufuntredrueckung")

    // send master request
    LIN_master3.sendMasterRequest(id, numData, data);

    // advance to next message
    count++;

  } // count == 0

  else {

    // assemble frame data
    id      = 0x1B;
    numData = 8;

    // get slave status & decode data
    LIN_master3.receiveSlaveResponse(id, numData, statusDecode);

    // restart LIN scheduler
    count=0;

  } // count == 1
  
} // LIN_scheduler()



// callback function to decode slave response signals
void statusDecode(uint8_t numData, uint8_t *data)
{
  // extract status signals from slave response frame (
  CoolFan_RPM_Ack             = data[0] & 0x7F;         // target speed [%Nnom]=[25rpm]
  CoolFan_EmAct_Stat          = bitRead(data[0],7);     // status emergency mode
  CoolFan_RPM_Avg             = data[1] & 0x7F;         // actual speed [%Nnom]=[25rpm]
  CoolFan_OvrVolt_Stat        = bitRead(data[1],7);     // status overvoltage shutdown
  CoolFan_Voltage_Avg         = data[2] & 0x7F;         // supply voltage [0.2V]
  CoolFan_UnderVolt_Stat      = bitRead(data[2],7);     // status undervoltage shutdown
  CoolFan_Current_Avg         = data[3] & 0x7F;         // supply current [A]
  CoolFan_VoltDerat_Stat      = bitRead(data[3],7);     // status voltage derating
  CoolFan_cur_Temp            = data[4];                // PCB temperature [C]
  CoolFan_TempDerat_Stat      = bitRead(data[5],0);     // status temperature derating
  CoolFan_Stiff_Stat          = bitRead(data[5],1);     // status sluggishness detection
  CoolFan_Blocking_Stat       = bitRead(data[5],2);     // status blocking detection
  CoolFan_Electr_Err          = bitRead(data[5],3);     // status internal error
  CoolFan_Mech_Err            = bitRead(data[5],4);     // status mechanical error
  CoolFan_OvrTemp_Err         = bitRead(data[5],5);     // status overtemperature
  CoolFan_Err_Group_ERR_Stat  = bitRead(data[5],6);     // collective error status 
  CoolFan_Err_Group_SNA_Stat  = bitRead(data[5],7);     // status Tpcb or ADC error
  CoolFan_Type                = data[6] & 0x0F;         // cooler fan type
  RsErr_CF                    = bitRead(data[7],5);     // ?
  WakeupStat_CF               = (data[7] & 0xC0) >> 6;  // ?

} // statusDecode()



// print slave response signals. Periodically called by task scheduler
void printStatus(void)
{
  // no LIN frame is ongoing -> print data
  if (LIN_master3.getState() == LIN_STATE_IDLE)
  {
    // LIN ok -> print some slave data
    if (LIN_master3.error == LIN_SUCCESS)
    {
      Serial.print("set speed: "); Serial.print(CoolFan_RPM_Ack*25);      Serial.println("rpm");
      Serial.print("act speed: "); Serial.print(CoolFan_RPM_Avg*25);      Serial.println("rpm");
      Serial.print("voltage:   "); Serial.print(CoolFan_Voltage_Avg*0.2); Serial.println("V");
      Serial.print("voltage:   "); Serial.print(CoolFan_Current_Avg);     Serial.println("A");
      Serial.print("voltage:   "); Serial.print((int)CoolFan_cur_Temp-0); Serial.println("C");
      Serial.print("blocking:  "); Serial.print((int)CoolFan_Blocking_Stat-0); Serial.println();
      Serial.println();
    }
    
    // print LIN error status
    else {
      Serial.print("LIN error (0x");
      Serial.print(LIN_master3.error, HEX);
      Serial.print("): ");
  
      if (LIN_master3.error & LIN_ERROR_STATE)
        Serial.print("statemachine ");
      
      if (LIN_master3.error & LIN_ERROR_ECHO)
        Serial.print("echo ");
      
      if (LIN_master3.error & LIN_ERROR_TIMEOUT)
        Serial.print("timeout ");
      
      if (LIN_master3.error & LIN_ERROR_CHK)
        Serial.print("checksum ");
      
      if (LIN_master3.error & LIN_ERROR_MISC)
        Serial.print("misc ");
  
      Serial.println();
  
    } // error
  
    // reset latched error and flag for data received
    LIN_master3.error = LIN_SUCCESS;

  } // LIN state == idle


  // LIN communication ongoing -> re-try in 2ms
  else
  {
    Tasks_Delay(printStatus, 2);
  }
    
} // printStatus()

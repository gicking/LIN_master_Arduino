/**
  \file     LIN_master0.cpp
  \brief    LIN master emulation library for Serial
  \details  This library provides a master node emulation for a LIN bus via Serial.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

// include files
#include "Arduino.h"
#include "LIN_master0.h"

// only compile if controller supports Serial
#if defined(HAVE_HWSERIAL0) || defined(SERIAL_PORT_HARDWARE)


/// instance of LIN master via Serial
LIN_Master_0     LIN_master0;


/**
  \brief      Constructor for LIN node class for Serial
  \details    Constructor for LIN node class for Serial. Store pointers used serial interface.
*/
LIN_Master_0::LIN_Master_0()
{
  // store used serial interface
  pSerial    = &Serial;       // store pointer to used serial
  #if defined(__AVR__)        // on AVR also store "double baudrate" control register (for sync break)
    UCSRA = &UCSR0A;
  #endif

  // store callback functions for task scheduler
  wrapperSend            = LIN_master0_send;
  wrapperReceive         = LIN_master0_receive;
  wrapperDefaultCallback = LIN_master0_copy;

  // for debug store class name for convenience
  #if (LIN_DEBUG_LEVEL != 0)
    sprintf(serialName, "LIN_Master_0");
  #endif

} // LIN_Master_0::LIN_Master_0()



/**
  \brief      Wrapper for LIN_master0 transmission handler
  \details    Wrapper for LIN_master0 transmission handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master0_send(void)
{
  // call class method
  LIN_master0.handlerSend();

} // LIN_master0_send



/**
  \brief      Wrapper for LIN_master0 reception handler
  \details    Wrapper for LIN_master0 reception handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master0_receive(void)
{
  // call class method
  LIN_master0.handlerReceive();

} // LIN_master0_receive



/**
  \brief      Wrapper for LIN_master0 default receive callback function
  \details    Wrapper for LIN_master0 default receive callback function for receiveSlaveResponse().
              Just copies data to a specified buffer.
*/
void LIN_master0_copy(uint8_t numData, uint8_t *data)
{
  // copy received data to user buffer
  LIN_master0.defaultCallback(numData, data);

} // LIN_master0_copy

#endif // HAVE_HWSERIAL0 || SERIAL_PORT_HARDWARE

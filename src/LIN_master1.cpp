/**
  \file     LIN_master1.cpp
  \brief    LIN master emulation library for Serial1
  \details  This library provides a master node emulation for a LIN bus via Serial1.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

// include files
#include "Arduino.h"
#include "LIN_master1.h"

// only compile if controller supports Serial1
#if defined(HAVE_HWSERIAL1) || defined(SERIAL_PORT_HARDWARE1)


/// instance of LIN master via Serial1
LIN_Master_1     LIN_master1;


/**
  \brief      Constructor for LIN node class for Serial1
  \details    Constructor for LIN node class for Serial1. Store pointers used serial interface.
*/
LIN_Master_1::LIN_Master_1()
{
  // store used serial interface
  pSerial    = &Serial1;      // store pointer to used serial
  #if defined(__AVR__)        // on AVR also store "double baudrate" control register (for sync break)
    UCSRA = &UCSR1A;
  #endif

  // store callback functions for task scheduler
  wrapperSend            = LIN_master1_send;
  wrapperReceive         = LIN_master1_receive;
  wrapperDefaultCallback = LIN_master1_copy;

  // for debug store class name for convenience
  #if (LIN_DEBUG_LEVEL != 0)
    sprintf(serialName, "LIN_Master_1");
  #endif

} // LIN_Master_1::LIN_Master_1()



/**
  \brief      Wrapper for LIN_master1 transmission handler
  \details    Wrapper for LIN_master1 transmission handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master1_send(void)
{
  // call class method
  LIN_master1.handlerSend();

} // LIN_master1_send



/**
  \brief      Wrapper for LIN_master1 reception handler
  \details    Wrapper for LIN_master1 reception handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master1_receive(void)
{
  // call class method
  LIN_master1.handlerReceive();

} // LIN_master1_receive



/**
  \brief      Wrapper for LIN_master1 default receive callback function
  \details    Wrapper for LIN_master1 default receive callback function for receiveSlaveResponse().
              Just copies data to a specified buffer.
*/
void LIN_master1_copy(uint8_t numData, uint8_t *data)
{
  // copy received data to user buffer
  LIN_master1.defaultCallback(numData, data);

} // LIN_master1_copy

#endif // HAVE_HWSERIAL1 || SERIAL_PORT_HARDWARE1

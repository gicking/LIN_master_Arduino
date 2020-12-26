/**
  \file     LIN_master2.cpp
  \brief    LIN master emulation library for Serial2
  \details  This library provides a master node emulation for a LIN bus via Serial2.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

// include files
#include "Arduino.h"
#include "LIN_master2.h"

// only compile if controller supports Serial2
#if defined(HAVE_HWSERIAL2) || defined(SERIAL_PORT_HARDWARE2)


/// instance of LIN master via Serial2
LIN_Master_2     LIN_master2;


/**
  \brief      Constructor for LIN node class for Serial2
  \details    Constructor for LIN node class for Serial2. Store pointers used serial interface.
*/
LIN_Master_2::LIN_Master_2()
{
  // store used serial interface
  pSerial    = &Serial2;      // store pointer to used serial
  #if defined(__AVR__)        // on AVR also store "double baudrate" control register (for sync break)
    UCSRA = &UCSR2A;
  #endif

  // store callback functions for task scheduler
  wrapperSend            = LIN_master2_send;
  wrapperReceive         = LIN_master2_receive;
  wrapperDefaultCallback = LIN_master2_copy;

  // for debug store class name for convenience
  #if (LIN_DEBUG_LEVEL != 0)
    sprintf(serialName, "LIN_Master_2");
  #endif

} // LIN_Master_2::LIN_Master_2()



/**
  \brief      Wrapper for LIN_master2 transmission handler
  \details    Wrapper for LIN_master2 transmission handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master2_send(void)
{
  // call class method
  LIN_master2.handlerSend();

} // LIN_master2_send



/**
  \brief      Wrapper for LIN_master2 reception handler
  \details    Wrapper for LIN_master2 reception handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master2_receive(void)
{
  // call class method
  LIN_master2.handlerReceive();

} // LIN_master2_receive



/**
  \brief      Wrapper for LIN_master2 default receive callback function
  \details    Wrapper for LIN_master2 default receive callback function for receiveSlaveResponse().
              Just copies data to a specified buffer.
*/
void LIN_master2_copy(uint8_t numData, uint8_t *data)
{
  // copy received data to user buffer
  LIN_master2.defaultCallback(numData, data);

} // LIN_master2_copy

#endif // HAVE_HWSERIAL2 || SERIAL_PORT_HARDWARE2

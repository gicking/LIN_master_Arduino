/**
  \file     LIN_master3.cpp
  \brief    LIN master emulation library for Serial3
  \details  This library provides a master node emulation for a LIN bus via Serial3.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

// include files
#include "Arduino.h"
#include "LIN_master3.h"

// only compile if controller supports Serial3
#if defined(HAVE_HWSERIAL3) || defined(SERIAL_PORT_HARDWARE3)


/// instance of LIN master via Serial3
LIN_Master_3     LIN_master3;


/**
  \brief      Constructor for LIN node class for Serial3
  \details    Constructor for LIN node class for Serial3. Store pointers used serial interface.
*/
LIN_Master_3::LIN_Master_3()
{
  // store used serial interface
  pSerial    = &Serial3;      // store pointer to used serial
  #if defined(__AVR__)        // on AVR also store "double baudrate" control register (for sync break)
    UCSRA = &UCSR3A;
  #endif

  // store callback functions for task scheduler
  wrapperSend            = LIN_master3_send;
  wrapperReceive         = LIN_master3_receive;
  wrapperDefaultCallback = LIN_master3_copy;

  // for debug store class name for convenience
  #if (LIN_DEBUG_LEVEL != 0)
    sprintf(serialName, "LIN_Master_3");
  #endif

} // LIN_Master_3::LIN_Master_3()



/**
  \brief      Wrapper for LIN_master3 transmission handler
  \details    Wrapper for LIN_master3 transmission handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master3_send(void)
{
  // call class method
  LIN_master3.handlerSend();

} // LIN_master3_send



/**
  \brief      Wrapper for LIN_master3 reception handler
  \details    Wrapper for LIN_master3 reception handler. This is required for
              task scheduler access to non-static member functions.
*/
void LIN_master3_receive(void)
{
  // call class method
  LIN_master3.handlerReceive();

} // LIN_master3_receive



/**
  \brief      Wrapper for LIN_master3 default receive callback function
  \details    Wrapper for LIN_master3 default receive callback function for receiveSlaveResponse().
              Just copies data to a specified buffer.
*/
void LIN_master3_copy(uint8_t numData, uint8_t *data)
{
  // copy received data to user buffer
  LIN_master3.defaultCallback(numData, data);

} // LIN_master3_copy

#endif // HAVE_HWSERIAL3 || SERIAL_PORT_HARDWARE3

/**
  \file     LIN_master0.h
  \brief    LIN master emulation library for Serial
  \details  This library provides a master node emulation for a LIN bus via Serial.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_MASTER0_H_
#define _LIN_MASTER0_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include "Arduino.h"
#include "LIN_master.h"


// only compile if controller supports Serial
#if defined(HAVE_HWSERIAL0) || defined(SERIAL_PORT_HARDWARE)


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/
/**
  \brief  LIN master node class

  \details LIN master node class. For background operation use wrapper functions.
*/
class LIN_Master_0 : public LIN_Master
{
  public:

    // public methods
    LIN_Master_0();                                         //!< class constructor
};

// external reference to LIN_master0
extern LIN_Master_0     LIN_master0;

/// Wrapper for LIN_master0 transmission handler
void LIN_master0_send(void);

/// Wrapper for LIN_master0 reception handler
void LIN_master0_receive(void);

/// Wrapper for LIN_master0 default receive callback function
void LIN_master0_copy(uint8_t numData, uint8_t *data);

#endif // HAVE_HWSERIAL0 || SERIAL_PORT_HARDWARE

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_MASTER0_H_

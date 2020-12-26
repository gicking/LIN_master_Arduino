/**
  \file     LIN_master3.h
  \brief    LIN master emulation library for Serial3
  \details  This library provides a master node emulation for a LIN bus via Serial3.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_MASTER3_H_
#define _LIN_MASTER3_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include "Arduino.h"
#include "LIN_master.h"


// only compile if controller supports Serial3
#if defined(HAVE_HWSERIAL3) || defined(SERIAL_PORT_HARDWARE3)


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/
/**
  \brief  LIN master node class

  \details LIN master node class. For background operation use wrapper functions.
*/
class LIN_Master_3 : public LIN_Master
{
  public:

    // public methods
    LIN_Master_3();                                         //!< class constructor
};

// external reference to LIN_master3
extern LIN_Master_3     LIN_master3;

/// Wrapper for LIN_master3 transmission handler
void LIN_master3_send(void);

/// Wrapper for LIN_master3 reception handler
void LIN_master3_receive(void);

/// Wrapper for LIN_master3 default receive callback function
void LIN_master3_copy(uint8_t numData, uint8_t *data);

#endif // HAVE_HWSERIAL3 || SERIAL_PORT_HARDWARE3

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_MASTER3_H_

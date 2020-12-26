/**
  \file     LIN_master1.h
  \brief    LIN master emulation library for Serial1
  \details  This library provides a master node emulation for a LIN bus via Serial1.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_MASTER1_H_
#define _LIN_MASTER1_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include "Arduino.h"
#include "LIN_master.h"


// only compile if controller supports Serial1
#if defined(HAVE_HWSERIAL1) || defined(SERIAL_PORT_HARDWARE1)


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/
/**
  \brief  LIN master node class

  \details LIN master node class. For background operation use wrapper functions.
*/
class LIN_Master_1 : public LIN_Master
{
  public:

    // public methods
    LIN_Master_1();                                         //!< class constructor
};

// external reference to LIN_master1
extern LIN_Master_1     LIN_master1;

/// Wrapper for LIN_master1 transmission handler
void LIN_master1_send(void);

/// Wrapper for LIN_master1 reception handler
void LIN_master1_receive(void);

/// Wrapper for LIN_master1 default receive callback function
void LIN_master1_copy(uint8_t numData, uint8_t *data);

#endif // HAVE_HWSERIAL1 || SERIAL_PORT_HARDWARE1

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_MASTER1_H_

/**
  \file     LIN_master2.h
  \brief    LIN master emulation library for Serial2
  \details  This library provides a master node emulation for a LIN bus via Serial2.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_MASTER2_H_
#define _LIN_MASTER2_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include "Arduino.h"
#include "LIN_master.h"


// only compile if controller supports Serial2
#if defined(HAVE_HWSERIAL2) || defined(SERIAL_PORT_HARDWARE2)


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/
/**
  \brief  LIN master node class

  \details LIN master node class. For background operation use wrapper functions.
*/
class LIN_Master_2 : public LIN_Master
{
  public:

    // public methods
    LIN_Master_2();                                         //!< class constructor
};

// external reference to LIN_master2
extern LIN_Master_2     LIN_master2;

/// Wrapper for LIN_master2 transmission handler
void LIN_master2_send(void);

/// Wrapper for LIN_master2 reception handler
void LIN_master2_receive(void);

/// Wrapper for LIN_master2 default receive callback function
void LIN_master2_copy(uint8_t numData, uint8_t *data);

#endif // HAVE_HWSERIAL2 || SERIAL_PORT_HARDWARE2

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_MASTER2_H_

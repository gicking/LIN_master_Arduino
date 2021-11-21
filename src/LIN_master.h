/**
  \file     LIN_master.h
  \brief    Base class for LIN master emulation
  \details  This library provides the base class for a master node emulation of a LIN bus.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_MASTER_H_
#define _LIN_MASTER_H_


/*-----------------------------------------------------------------------------
  GLOBAL DEFINES
-----------------------------------------------------------------------------*/

#define LIN_DEBUG_SERIAL   Serial       //!< Serial interface used for debug output
#define LIN_DEBUG_LEVEL    0            //!< Debug level (0=no output, 1=error msg, 2=sent/received bytes)


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// Check library dependencies
#if defined(__AVR__) //the __has_include() macro only kind of works at least with the Arduino MEGA
    #if !__has_include("../../Task_Scheduler/src/Tasks.h")
        #error This LIN library requires the "Task Scheduler" library by Kai Liebich & Georg Icking-Konert. Please install it via the Arduino library manager!
    #endif
#endif

// include required libs
#include "Arduino.h"
#include "Tasks.h"


/*-----------------------------------------------------------------------------
        GLOBAL ENUMS/STRUCTS
-----------------------------------------------------------------------------*/

/**
    \brief LIN version of checksum
*/
typedef enum {
    LIN_V1            = 1,          //!< LIN protocol version 1
    LIN_V2            = 2           //!< LIN protocol version 2
} LIN_version_t;


/**
    \brief LIN frame type
*/
typedef enum {
    LIN_MASTER_REQUEST = 1,         //!< LIN protocol version 1
    LIN_SLAVE_RESPONSE = 2          //!< LIN protocol version 2
} LIN_frame_t;


/**
    \brief LIN communication error codes
*/
typedef enum {
    LIN_SUCCESS       = 0x00,       //!< no error
    LIN_ERROR_STATE   = 0x01,       //!< error in LIN state machine
    LIN_ERROR_ECHO    = 0x02,       //!< error reading LIN echo
    LIN_ERROR_TIMEOUT = 0x04,       //!< LIN receive timeout
    LIN_ERROR_CHK     = 0x08,       //!< LIN checksum error
    LIN_ERROR_MISC    = 0x80        //!< misc error, should not occur
} LIN_error_t;


/**
    \brief state of LIN master state machine
*/
typedef enum {
    LIN_STATE_OFF     = 0,          //!< LIN instance inactive
    LIN_STATE_IDLE    = 1,          //!< no LIN transmission ongoing
    LIN_STATE_BREAK   = 2,          //!< break is being transmitted
    LIN_STATE_FRAME   = 3,          //!< frame is being transmitted
} LIN_status_t;


/**
    \brief typedef for data decoder to hadle received data
*/
typedef void (*decoder_t)(uint8_t,uint8_t*);



/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/

/**
  \brief  LIN master node base class

  \details LIN master node base class. From this class the actual LIN classes for a Serialx are derived.
*/
class LIN_Master
{
  protected:

    // internal variables
    HardwareSerial    *pSerial;                                               //!< pointer to used serial
    #if defined(__AVR__)
      volatile uint8_t  *UCSRA;                                               //!< "double baudrate" control register on AVR
    #endif
    void              (*wrapperSend)(void);                                   //!< wrapper for transmission handler (for task scheduler)
    void              (*wrapperReceive)(void);                                //!< wrapper for reception handler (for task scheduler)
    void              (*wrapperDefaultCallback)(uint8_t, uint8_t*);           //!< wrapper for default receive callback function
    #if (LIN_DEBUG_LEVEL != 0)
      char            serialName[20];                                         //!< for debug store class name for convenience
    #endif
    uint16_t          baudrate;                                               //!< communication baudrate [Baud]
    LIN_version_t     version;                                                //!< LIN version for checksum calculation
    bool              background;                                             //!< background or blocking operation
    uint8_t           durationBreak;                                          //!< duration of sync break [ms]
    LIN_frame_t       frameType;                                              //!< LIN frame type
    uint8_t           bufTx[12];                                              //!< send buffer incl. BREAK, SYNC, DATA and CHK (max. 12B)
    uint8_t           lenTx;                                                  //!< send buffer length (max. 12)
    uint8_t           bufRx[12];                                              //!< receive buffer incl. SYNC, DATA and CHK (max. 11B)
    uint8_t           lenRx;                                                  //!< receive buffer length (max. 12)
    uint8_t           durationFrame;                                          //!< duration of frame w/o BREAK [ms]
    LIN_status_t      state;                                                  //!< status of LIN state machine
    void              (*rx_handler)(uint8_t, uint8_t*);                       //!< handler to decode slave response (for receiveFrame())
    uint8_t           *dataPtr;                                               //!< pointer to data buffer in LIN_master3_copy()

    // internal methods
    uint8_t           protectID(uint8_t id);                                  //!< calculate protected LIN ID
    uint8_t           checksum(uint8_t id, uint8_t numData, uint8_t *data);   //!< calculate frame checksum


  public:

    // public variables
    LIN_error_t       error;                                                  //!< error state. Is latched until cleared

    // public methods
    void              begin(uint16_t Baudrate, LIN_version_t Version, bool Background);  //!< setup UART and LIN framework
    void              end(void);                                              //!< end UART communication    void              end(void);                                                         //!< end UART communication
    LIN_error_t       sendMasterRequest(uint8_t id, uint8_t numData, uint8_t *data);     //!< send a master request frame
    LIN_error_t       receiveSlaveResponse(uint8_t id, uint8_t numData, void (*Rx_handler)(uint8_t, uint8_t*));  //!< receive a slave response frame with callback function
    LIN_error_t       receiveSlaveResponse(uint8_t id, uint8_t numData, uint8_t *data);  //!< receive a slave response frame and copy to buffer
    inline bool       getState(void) { return state; }                        //!< get state of LIN handler machine state. For use see example LIN_background.ino 

    /// LIN master receive handler for task scheduler. Must be public for task scheduler, but must NOT be used by user program!
    void              handlerSend(void);                                      //!< send handler for task scheduler
    void              handlerReceive(void);                                   //!< send handler for task scheduler
    void              defaultCallback(uint8_t numData, uint8_t *data);        //!< receive callback function to copy data to buffer

};

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_MASTER_H_

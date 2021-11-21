/**
  \file     LIN_master.cpp
  \brief    Base class for LIN master emulation
  \details  This library provides the base class for a master node emulation of a LIN bus.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
  \date     2020-03-14
  \version  0.1
*/

// include files
#include "Arduino.h"
#include "LIN_master.h"


/**
  \brief      Setup library for LIN communication.
  \details    Create an instance of LIN master, configure UART and store bus parameters.
              For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \param[in]  Baudrate    communication baudrate [Baud]. Default is 19200 Baud
  \param[in]  Version     LIN version for checksum calculation. Default is LIN_V2
  \param[in]  Background  background or blocking operation
*/
void LIN_Master::begin(uint16_t Baudrate=19200, LIN_version_t Version=LIN_V2, bool Background=true)
{
  // store parameters in class
  baudrate   = Baudrate;      // communication baudrate [Baud]
  version    = Version;       // LIN version for checksum calculation
  background = Background;    // background or blocking communication
  if (baudrate < 12000) {     // (rough) duration of sync break [ms] and max. frame w/o break [ms]
    durationBreak = 2;
    durationFrame = 13;
  }
  else {
    durationBreak = 1;
    durationFrame = 7;
  }

  // reset internal variables
  error = LIN_SUCCESS;       // last LIN error. Is latched
  state = LIN_STATE_IDLE;    // status of LIN state machine

  // initialize serial interface
  pSerial->begin(Baudrate); while(!(*pSerial));

  // set low timeout to avoid bus blocking
  pSerial->setTimeout(2);

} // LIN_Master::begin()



/**
  \brief      Close serial communication.
  \details    Close serial communication, reset internal status
*/
void LIN_Master::end() {

  // reset internal variables
  error = LIN_SUCCESS;     // revert LIN error
  state = LIN_STATE_OFF;   // revert LIN state machine

  // close Serial3
  pSerial->end();

  // restore default timeout
  pSerial->setTimeout(1000);

} // LIN_Master::end()



/**
  \brief      Calculate protected LIN ID.
  \details    Method to calculate the protected LIN identifier as described in LIN2.0 spec "2.3.1.3 Protected identifier field"
  \param[in]  id      frame ID (protection optional)
  \return     protected LIN identifier
*/
uint8_t LIN_Master::protectID(uint8_t id)
{
  uint8_t  pid;       // result = protected ID
  uint8_t  tmp;       // temporary variable for calculating parity bits

  // copy (unprotected) ID
  pid = id;

  // protect ID  with parity bits
  pid  = (uint8_t) (pid & 0x3F);                                          // clear upper bit 6&7
  tmp  = (uint8_t) ((pid ^ (pid>>1) ^ (pid>>2) ^ (pid>>4)) & 0x01);       // -> pid[6] = PI0 = ID0^ID1^ID2^ID4
  pid |= (uint8_t) (tmp << 6);
  tmp  = (uint8_t) (~((pid>>1) ^ (pid>>3) ^ (pid>>4) ^ (pid>>5)) & 0x01); // -> pid[6] = PI1 = ~(ID1^ID3^ID4^ID5)
  pid |= (uint8_t) (tmp << 7);

  // return protected identifier
  return pid;

} // LIN_Master::protectID()



/**
  \brief      Calculate LIN frame checksum.
  \details    Method to calculate the LIN frame checksum as described in LIN2.0 spec
  \param[in]  id          frame ID
  \param[in]  numData     number of data bytes in frame
  \param[in]  data        buffer containing data bytes
  \return     calculated frame checksum
*/
uint8_t LIN_Master::checksum(uint8_t id, uint8_t numData, uint8_t *data)
{
  uint16_t chk=0x00;

  // protect the ID
  id = protectID(id);

  // LIN2.x uses extended checksum which includes protected ID, i.e. including parity bits
  // LIN1.x uses classical checksum only over data bytes
  // Diagnostic frames with ID 0x3C and 0x3D/0x7D always use classical checksum (see LIN spec "2.3.1.5 Checkum")
  if (!((version == LIN_V1) || (id == 0x3C) || (id == 0x7D)))    // if version 2  & no diagnostic frames (0x3C=60 (PID=0x3C) or 0x3D=61 (PID=0x7D))
    chk = (uint16_t) id;

  // loop over data bytes
  for (uint8_t i = 0; i < numData; i++)
  {
    chk += (uint16_t) (data[i]);
    if (chk>255)
      chk -= 255;
  }
  chk = (uint8_t)(0xFF - ((uint8_t) chk));   // bitwise invert

  // return frame checksum
  return chk;

} // LIN_Master::checksum()



/**
  \brief      send a master request frame.
  \details    Send a master request frame. Actual transmission is handled by task scheduler for background operation.
              For an explanation of the LIN bus and protocoll e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network.
  \param[in]  id          frame ID (protection optional)
  \param[in]  numData     number of data bytes (0..8)
  \param[in]  data        Tx data bytes
*/
LIN_error_t LIN_Master::sendMasterRequest(uint8_t id, uint8_t numData, uint8_t *data)
{
  // return immediately if LIN state machine not in idle state
  if (state != LIN_STATE_IDLE)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".sendMasterRequest(): state != LIN_STATE_IDLE (is ");
      LIN_DEBUG_SERIAL.print(state);
      LIN_DEBUG_SERIAL.println(")");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_STATE);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return LIN_ERROR_STATE;
  }

  // set master request frame type
  frameType = LIN_MASTER_REQUEST;

  // protect ID
  id = protectID(id);

  // construct frame: BREAK + SYNC + ID + DATA + CHK. Note: BREAK is handled outside
  bufTx[0] = 0x00;                                // sync break
  bufTx[1] = 0x55;                                // sync field
  bufTx[2] = id;                                  // protected ID
  memcpy(bufTx+3, data, numData);                 // data bytes
  bufTx[3+numData] = checksum(id, numData, data); // frame checksum
  lenTx = numData+4;                              // number of bytes to send (BREAK + SYNC + ID + DATA + CHK)
  lenRx = lenTx;                                  // number of bytes to receive (BREAK + SYNC + ID + DATA + CHK)

  // for printing data to send, set debug level >=2
  #if (LIN_DEBUG_LEVEL >= 2)
    LIN_DEBUG_SERIAL.print(millis());
    LIN_DEBUG_SERIAL.print("ms ");
    LIN_DEBUG_SERIAL.print(serialName);
    LIN_DEBUG_SERIAL.print(".sendMasterRequest(): send ");
    LIN_DEBUG_SERIAL.print(lenTx);
    LIN_DEBUG_SERIAL.print(" bytes");
    for (uint8_t i=0; i<lenTx; i++)
    {
      LIN_DEBUG_SERIAL.print(" 0x");
      LIN_DEBUG_SERIAL.print(bufTx[i], HEX);
    }
    LIN_DEBUG_SERIAL.println();
  #endif

  // clear receive buffer (required to recover from error)
  while (pSerial->available())
    pSerial->read();

  // set half baudrate for LIN break
  #if defined(__AVR__)
    *UCSRA &= ~(1<<U2X0);                              // on AVR clear "double baudrate"
  #else
    pSerial->begin(baudrate/2); while(!(*pSerial));    // else use built-in function
  #endif

  // send sync break (=0x00 at 1/2 baudrate)
  pSerial->write((uint8_t) bufTx[0]);

  // set new state of LIN state machine
  state = LIN_STATE_BREAK;


  // background operation -> use task scheduler
  if (background)
  {
    // attach send handler for frame body
    Tasks_Add((Task) wrapperSend, 0, durationBreak);

  } // background operation

  // blocking operation -> call handlers manually
  else
  {
    // wait until break has been sent
    pSerial->flush();

    // call send handler manually
    wrapperSend();

    // wait until data has been sent
    pSerial->flush();

    // call receive handler manually
    wrapperReceive();

  } // blocking operation

} // LIN_Master::sendMasterRequest



/**
  \brief      Receive slave response frame with callback function
  \details    Receive a slave response and use callback function for handling received data.
              Actual transmission is handled by task scheduler for background operation.
              For an explanation of the LIN bus and protocoll e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network.
  \param[in]  id          frame ID (protection optional)
  \param[in]  numData     number of data bytes (0..8)
  \param[out] Rx_handler  callback function to handle received data
*/
LIN_error_t LIN_Master::receiveSlaveResponse(uint8_t id, uint8_t numData, void (*Rx_handler)(uint8_t, uint8_t*))
{
  // return immediately if LIN state machine not in idle state
  if (state != LIN_STATE_IDLE)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".receiveSlaveResponse(): state != LIN_STATE_IDLE (is ");
      LIN_DEBUG_SERIAL.print(state);
      LIN_DEBUG_SERIAL.println(")");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_STATE);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return LIN_ERROR_STATE;
  }

  // set slave response frame type
  frameType = LIN_SLAVE_RESPONSE;

  // protect ID
  id = protectID(id);

  // construct frame: BREAK + SYNC + ID. Note: BREAK is handled outside
  bufTx[0] = 0x00;                                // sync break
  bufTx[1] = 0x55;                                // sync field
  bufTx[2] = id;                                  // protected ID
  lenTx = 3;                                      // number of bytes to send (BREAK + SYNC + ID)
  lenRx = 4 + numData;                            // number of bytes to receive (BREAK + SYNC + ID + DATA + CHK)

  // for printing data to send, set debug level >=2
  #if (LIN_DEBUG_LEVEL >= 2)
    LIN_DEBUG_SERIAL.print(millis());
    LIN_DEBUG_SERIAL.print("ms ");
    LIN_DEBUG_SERIAL.print(serialName);
    LIN_DEBUG_SERIAL.print(".receiveSlaveResponse(): send ");
    LIN_DEBUG_SERIAL.print(lenTx);
    LIN_DEBUG_SERIAL.print(" bytes");
    for (uint8_t i=0; i<lenTx; i++)
    {
      LIN_DEBUG_SERIAL.print(" 0x");
      LIN_DEBUG_SERIAL.print(bufTx[i], HEX);
    }
    LIN_DEBUG_SERIAL.println();
  #endif

  // clear receive buffer (required to recover from error)
  while (pSerial->available())
    pSerial->read();

  // set half baudrate for LIN break
  #if defined(__AVR__)
    *UCSRA &= ~(1<<U2X0);                              // on AVR clear "double baudrate"
  #else
    pSerial->begin(baudrate/2); while(!(*pSerial));    // else use built-in function
  #endif

  // set callback function to handle received bytes when finished
  rx_handler = Rx_handler;

  // send sync break (=0x00 at 1/2 baudrate)
  pSerial->write((uint8_t) bufTx[0]);

  // set new state of LIN state machine
  state = LIN_STATE_BREAK;


  // background operation -> use task scheduler
  if (background)
  {
    // attach send handler for frame body
    Tasks_Add((Task) wrapperSend, 0, durationBreak);

  } // background operation

  // blocking operation -> call handlers manually
  else
  {
    // wait until break has been sent
    pSerial->flush();

    // call send handler manually
    wrapperSend();

    // wait until slave has responded (with timeout)
    uint32_t tStart = millis();
    while ((pSerial->available() != lenRx-1) && ((millis() - tStart) < durationFrame));

    // call receive handler manually
    wrapperReceive();

  } // blocking operation

} // LIN_Master::receiveSlaveResponse (callback)



/**
  \brief      Receive slave response frame and copy to buffer
  \details    Receive a slave response and copy received data to specified buffer after reception.
              Actual transmission is handled by task scheduler for background operation.
              For an explanation of the LIN bus and protocoll e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network.
  \param[in]  id          frame ID (protection optional)
  \param[in]  numData     number of data bytes (0..8)
  \param[out] data        buffer to copy data to fater reception
*/
LIN_error_t LIN_Master::receiveSlaveResponse(uint8_t id, uint8_t numData, uint8_t *data)
{
  // store array pointer for default callback function
  dataPtr = data;

  // call receive function with callback for actual transmission
  receiveSlaveResponse(id, numData, wrapperDefaultCallback);

} // LIN_Master::receiveSlaveResponse (copy data)



/**
  \brief      Handler for LIN master transmission
  \details    Handler for LIN master transmission. Here the remainder of the frame after sync break is sent.
*/
void LIN_Master::handlerSend(void)
{
  // check state of state machine
  if (state != LIN_STATE_BREAK)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".handlerSend(): state != LIN_STATE_BREAK (is ");
      LIN_DEBUG_SERIAL.print(state);
      LIN_DEBUG_SERIAL.println(")");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_STATE);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return;
  }


  // wait until break received (with timeout) before changing baudrate
  uint32_t tStart = micros();
  while ((!(pSerial->available())) && ((micros() - tStart) < 500));


  // assert no timeout
  if (!(pSerial->available()))
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.println(".handlerSend(): receive BREAK timeout");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_TIMEOUT);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return;
  }

  // assert correct echo
  bufRx[0] = pSerial->read();
  if (bufRx[0] != 0x00)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".handlerSend(): received BREAK != 0x00 (is ");
      LIN_DEBUG_SERIAL.print(bufRx[0]);
      LIN_DEBUG_SERIAL.println(")");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_ECHO);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return;
  }

  // BREAK echo read successfully
  else
  {
    #if (LIN_DEBUG_LEVEL >= 2)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.println(".handlerSend(): received BREAK echo");
    #endif
  }


  // restore original baudrate after BREAK
  #if defined(__AVR__)
    *UCSRA |= (1<<U2X0);                               // on AVR restore "double baudrate"
  #else
    pSerial->begin(baudrate); while(!(*pSerial));      // else use built-in function
  #endif

  // write remainder of frame or header
  pSerial->write(bufTx+1, lenTx-1);

  // set new state of LIN state machine
  state = LIN_STATE_FRAME;

  // background operation
  if (background)
  {
    // attach receive handler for reading frame echo or header echo + slave response
    Tasks_Add((Task) wrapperReceive, 0, durationFrame);

  } // background operation

} // LIN_Master::handlerSend



/**
  \brief      Handler for LIN_master3 transmission
  \details    Handler for LIN_master3 transmission. This wrapper is required for
              task scheduler access to non-static member functions and class variables.
*/
void LIN_Master::handlerReceive(void)
{
  uint8_t   i;

  // check state of state machine
  if (state != LIN_STATE_FRAME)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".handlerReceive: state != LIN_STATE_FRAME (is ");
      LIN_DEBUG_SERIAL.print(state);
      LIN_DEBUG_SERIAL.println(")");
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_STATE);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return;
  }


  // wait until break received (with timeout) before changing baudrate
  uint32_t tStart = micros();
  while ((pSerial->available() != lenRx-1) && ((micros() - tStart) < 500));


  // check if data was received (-1 because sync break already read)
  if (pSerial->available() != lenRx-1)
  {
    // for printing error message, set debug level >=1
    #if (LIN_DEBUG_LEVEL >= 1)
      uint8_t num = pSerial->available();
      LIN_DEBUG_SERIAL.print(millis());
      LIN_DEBUG_SERIAL.print("ms ");
      LIN_DEBUG_SERIAL.print(serialName);
      LIN_DEBUG_SERIAL.print(".handlerReceive: receive frame timeout (");
      LIN_DEBUG_SERIAL.print(num+1); LIN_DEBUG_SERIAL.print(" vs. "); LIN_DEBUG_SERIAL.print(lenRx);
      LIN_DEBUG_SERIAL.println(")");
      /*
      for (i=0; i<num+1; i++)
      {
        LIN_DEBUG_SERIAL.print("  ");
        LIN_DEBUG_SERIAL.print(i);
        LIN_DEBUG_SERIAL.print(": 0x");
        LIN_DEBUG_SERIAL.println((uint8_t) (bufRx[i]), HEX);
      }
      */
    #endif
    error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_TIMEOUT);
    state = LIN_STATE_IDLE;
    memset(bufRx, 0, lenRx);
    return;
  }


  // copy received bytes to LIN buffer
  for (i=1; i<lenRx; i++)
    bufRx[i] = pSerial->read();


  // for master request FRAME just check LIN echo
  if (frameType == LIN_MASTER_REQUEST)
  {
    // check if sent and received data match
    if (memcmp(bufRx, bufTx, lenTx) != 0)
    {
      // for printing error message, set debug level >=1
      #if (LIN_DEBUG_LEVEL >= 1)
        LIN_DEBUG_SERIAL.print(millis());
        LIN_DEBUG_SERIAL.print("ms ");
        LIN_DEBUG_SERIAL.print(serialName);
        LIN_DEBUG_SERIAL.print(".handlerReceive: LIN frame echo mismatch:");
        for (i=0; i<lenRx; i++)
        {
          LIN_DEBUG_SERIAL.print("  ");
          LIN_DEBUG_SERIAL.print(i);
          LIN_DEBUG_SERIAL.print(": 0x"); LIN_DEBUG_SERIAL.print((uint8_t) (bufRx[i]), HEX);
          LIN_DEBUG_SERIAL.print(" vs. ");
          LIN_DEBUG_SERIAL.print(" 0x"); LIN_DEBUG_SERIAL.print((uint8_t) (bufTx[i]), HEX);
          LIN_DEBUG_SERIAL.println();
        }
      #endif
      error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_ECHO);
      state = LIN_STATE_IDLE;
      memset(bufRx, 0, lenRx);
      return;
    } // frame echo mismatch

    // frame echo read successfully
    else
    {
      #if (LIN_DEBUG_LEVEL >= 2)
        LIN_DEBUG_SERIAL.print(millis());
        LIN_DEBUG_SERIAL.print("ms ");
        LIN_DEBUG_SERIAL.print(serialName);
        LIN_DEBUG_SERIAL.println(".handlerReceive: received frame echo");
      #endif
    }

  } // LIN_MASTER_REQUEST


  // for slave response frame check header echo and frame checksum
  else
  {
    // check if sent and received frame headers (BREAK+SYNC+ID) match
    if (memcmp(bufRx, bufTx, 3) != 0) {
      // for printing error message, set debug level >=1
      #if (LIN_DEBUG_LEVEL >= 1)
        LIN_DEBUG_SERIAL.print(millis());
        LIN_DEBUG_SERIAL.print("ms ");
        LIN_DEBUG_SERIAL.print(serialName);
        LIN_DEBUG_SERIAL.print(".handlerReceive: LIN header echo mismatch:");
        for (i=0; i<3; i++)
        {
          LIN_DEBUG_SERIAL.print("  ");
          LIN_DEBUG_SERIAL.print(i);
          LIN_DEBUG_SERIAL.print(": 0x"); LIN_DEBUG_SERIAL.print((uint8_t) (bufRx[i]), HEX);
          LIN_DEBUG_SERIAL.print(" vs. ");
          LIN_DEBUG_SERIAL.print(" 0x"); LIN_DEBUG_SERIAL.print((uint8_t) (bufTx[i]), HEX);
          LIN_DEBUG_SERIAL.println();
        }
      #endif
      error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_ECHO);
      state = LIN_STATE_IDLE;
      memset(bufRx, 0, lenRx);
      return;
    } // header echo mismatch


    // assert checksum
    uint8_t  id      = bufRx[2];                      // frame ID
    uint8_t  numData = lenRx-4;                       // received length - (BREAK-SYNC+ID+CHK)
    uint8_t  *data   = bufRx+3;                       // pointer to start of data bytes
    uint8_t  chk     = bufRx[lenRx-1];    // received checksum
    uint8_t  chk_calc = checksum(id, numData, data);  // calculated checksum
    if (chk != chk_calc)
    {
      // for printing error message, set debug level >=1
      #if (LIN_DEBUG_LEVEL >= 1)
        LIN_DEBUG_SERIAL.print(millis());
        LIN_DEBUG_SERIAL.print("ms ");
        LIN_DEBUG_SERIAL.print(serialName);
        LIN_DEBUG_SERIAL.print(".handlerReceive: checksum error (");
        LIN_DEBUG_SERIAL.print(chk, HEX); LIN_DEBUG_SERIAL.print(" vs. "); LIN_DEBUG_SERIAL.print(chk_calc, HEX);
        LIN_DEBUG_SERIAL.println(")");
      #endif
      error = (LIN_error_t)((uint8_t) error | (uint8_t) LIN_ERROR_CHK);
      state = LIN_STATE_IDLE;
      memset(bufRx, 0, lenRx);
      return;
    } // checksum error

    // use callback function to handle received data. Only data bytes (- BREAK - SYNC - ID - CHK)
    rx_handler(lenRx-4, bufRx+3);

  } // LIN_SLAVE_RESPONSE


  // reset state of LIN state machine
  state = LIN_STATE_IDLE;

} // LIN_Master::handlerReceive



/**
  \brief      Receive handler to copy data to buffer
  \details    Dummy receive handler for receiveSlaveResponse() to copy data to a specified buffer.
              array pointer must be stored in dataPtr before calling
*/
void LIN_Master::defaultCallback(uint8_t numData, uint8_t *data)
{
  // copy received data to user buffer
  memcpy(dataPtr, data, numData);

} // LIN_Master::defaultCallback

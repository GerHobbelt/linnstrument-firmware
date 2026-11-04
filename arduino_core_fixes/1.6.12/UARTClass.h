/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _UART_CLASS_
#define _UART_CLASS_

#include "HardwareSerial.h"
#include "RingBuffer.h"

// Includes Atmel CMSIS
#include <chip.h>

#define SERIAL_8N1 UARTClass::Mode_8N1
#define SERIAL_8E1 UARTClass::Mode_8E1
#define SERIAL_8O1 UARTClass::Mode_8O1
#define SERIAL_8M1 UARTClass::Mode_8M1
#define SERIAL_8S1 UARTClass::Mode_8S1


class UARTClass : public HardwareSerial
{
  public:
    enum UARTModes {
      Mode_8N1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_NO,
      Mode_8E1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_EVEN,
      Mode_8O1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_ODD,
      Mode_8M1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_MARK,
      Mode_8S1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_SPACE,
    };
    UARTClass(Uart* pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer, RingBuffer* pTx_buffer);

    virtual void begin(const uint32_t dwBaudRate) override;
    void begin(const uint32_t dwBaudRate, const UARTModes config);
    virtual void end(void) override;
    virtual int available(void) override;
    int availableForWrite(void);
    virtual int peek(void) override;
    virtual int read(void) override;
    virtual void flush(void) override;
    virtual void drop(void) override;
    virtual bool isFlushed(void) override;
    virtual size_t write(const uint8_t c) override;  // spin locks until c has been sent.
    virtual bool write_if_possible(const uint8_t uc_data) override;    // return true if sent.

    using Print::write; // pull in write(str) and write(buf, size) from Print

    void setInterruptPriority(uint32_t priority);
    uint32_t getInterruptPriority();

    void IrqHandler(void);

  protected:
    volatile uint8_t error_state;  // bits: 2: RX buffer overflow; rest cf. SAM3X datasheet section 34.6.6 UART Status Register, bits 7..0
	uint8_t initialized;

  public:
    bool getOverflowed() {
      bool v = !!(error_state & 0x04);
      error_state &= ~0x04; // clear the bit
      return v;
    }
    void setOverflowed() {
      error_state |= 0x04; // set RX buffer overflow bit
    }
  protected:
    // error reporting outside ISR:
	void setUARTstatusBits(uint8_t errors) {
	  error_state |= errors & (UART_SR_OVRE | UART_SR_FRAME | UART_SR_PARE);
	}
  public:
    bool hasAnyError() {
	  return !!error_state;
	}
    uint8_t getAndClearAllErrors() {
	  uint8_t v = error_state;
	  error_state = 0;
	  return v;
	}
	bool getOverrunError() {
	  bool v = !!(error_state & UART_SR_OVRE);
	  error_state &= ~UART_SR_OVRE;
	  return v;
	}
	bool getFramingError() {
	  bool v = !!(error_state & UART_SR_FRAME);
	  error_state &= ~UART_SR_FRAME;
	  return v;
	}
	bool getParityError() {
	  bool v = !!(error_state & UART_SR_PARE);
	  error_state &= ~UART_SR_PARE;
	  return v;
	}
	
    operator bool() { return true; }; // UART always active

  protected:
    void init(const uint32_t dwBaudRate, const uint32_t config);

    RingBuffer *_rx_buffer;
    RingBuffer *_tx_buffer;

    Uart* _pUart;
    IRQn_Type _dwIrq;
    uint32_t _dwId;
};

#endif // _UART_CLASS_

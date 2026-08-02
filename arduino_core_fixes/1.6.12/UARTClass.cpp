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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "UARTClass.h"

// Constructors ////////////////////////////////////////////////////////////////

UARTClass::UARTClass( Uart *pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer *pRx_buffer, RingBuffer *pTx_buffer )
: error_state(0), initialized(0)
{
  _rx_buffer = pRx_buffer;
  _tx_buffer = pTx_buffer;

  _pUart = pUart;
  _dwIrq = dwIrq;
  _dwId = dwId;
}

// Public Methods //////////////////////////////////////////////////////////////

void UARTClass::begin(const uint32_t dwBaudRate)
{
  begin(dwBaudRate, Mode_8N1);
}

void UARTClass::begin(const uint32_t dwBaudRate, const UARTModes config)
{
  uint32_t modeReg = static_cast<uint32_t>(config) & 0x00000E00;
  init(dwBaudRate, modeReg | UART_MR_CHMODE_NORMAL);
}

void UARTClass::init(const uint32_t dwBaudRate, const uint32_t modeReg)
{
  initialized = 0;
  
  // Configure PMC
  pmc_enable_periph_clk( _dwId );

  // Disable PDC channel
  _pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  // Reset and disable receiver and transmitter; also reset any previous error flags, if any.
  _pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS | UART_CR_RSTSTA;

  // Configure mode
  _pUart->UART_MR = modeReg;

  // Configure baudrate (asynchronous, no oversampling, with rounding)
  _pUart->UART_BRGR = ((SystemCoreClock / dwBaudRate) + 8) >> 4;

  // Configure interrupts
  //
  // SAM3X datasheet says:
  //   On receipt of the interrupt signal, the CPU enters the interrupt handler (Refer to the Interrupt
  //   Controller). To ascertain which interrupt has been generated, read the interrupt status register. Note that this
  //   register clears itself when read. At reset, all interrupts are disabled. To enable an interrupt, write to interrupt enable
  //   register with the pertinent interrupt bit set to 1. To disable an interrupt, write to interrupt disable register with the
  //   pertinent interrupt bit set to 1. To check whether an interrupt is enabled or disabled, read interrupt mask register: if
  //   the bit is set to 1, the interrupt is disabled.
  //
  _pUart->UART_IDR = 0xFFFFFFFF;
  _pUart->UART_IER = UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME;

  // Enable UART interrupt in NVIC
  NVIC_EnableIRQ(_dwIrq);
  
  // Make sure both ring buffers are initialized back to empty.
  _rx_buffer->reset();
  _tx_buffer->reset();
  
  // nuke all previous errors, which may have occurred before we (re)initialized the UART:
  (void)getAndClearAllErrors();
  
  initialized = 1;

  // Enable receiver and transmitter
  _pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
}

void UARTClass::end( void )
{
  // Clear any received data
  _rx_buffer->drop();

  // Wait for any outstanding data to be sent
  flush();

  initialized = 0;
  
  // Disable UART interrupt in NVIC
  NVIC_DisableIRQ( _dwIrq );

  pmc_disable_periph_clk( _dwId );
}

void UARTClass::setInterruptPriority(uint32_t priority)
{
  NVIC_SetPriority(_dwIrq, priority & 0x0F);
}

uint32_t UARTClass::getInterruptPriority()
{
  return NVIC_GetPriority(_dwIrq);
}

int UARTClass::available( void )
{
  return _rx_buffer->available();
}

int UARTClass::availableForWrite(void)
{
  return _tx_buffer->available();
}

int UARTClass::peek( void )
{
  return _rx_buffer->peek_char();
}

int UARTClass::read( void )
{
  return _rx_buffer->read_char();
}

void UARTClass::flush( void )
{
  if (!initialized) {
    return drop();
  }
	
  _tx_buffer->flush();
	
  // Wait for transmission to complete
  while ((_pUart->UART_SR & UART_SR_TXEMPTY) != UART_SR_TXEMPTY)
    ;
}

void UARTClass::drop( void )
{
  _tx_buffer->drop();
}

bool UARTClass::isFlushed( void )
{
  return (!initialized || ((_pUart->UART_SR & UART_SR_TXRDY) == UART_SR_TXRDY)) && _tx_buffer->isFlushed();
}

size_t UARTClass::write( const uint8_t uc_data )
{
  if (!initialized) {
    return 0;
  }
  
  // Is the hardware currently busy?
  if (((_pUart->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY) ||
      !_tx_buffer->isFlushed())
  {
    // If busy we buffer
    bool rv;
	do {
	  rv = _tx_buffer->store_char( uc_data );
	} while (!rv);  // Spin locks if we're about to overwrite the buffer. This continues once the data is sent

    // Make sure TX interrupt is enabled
    _pUart->UART_IER = UART_IER_TXRDY;
  }
  else 
  {
     // Bypass buffering and send character directly
     _pUart->UART_THR = uc_data;
  }
  return 1;
}

bool UARTClass::write_if_possible( const uint8_t uc_data )
{
  if (!initialized) {
    return false;
  }
  
  // Is the hardware currently busy?
  if (((_pUart->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY) |
      !_tx_buffer->isFlushed())
  {
    // If busy we buffer
    bool rv = _tx_buffer->store_char( uc_data );

	if (rv) {
      // Make sure TX interrupt is enabled
      _pUart->UART_IER = UART_IER_TXRDY;
	}
	
	return rv;
  }
  else 
  {
     // Bypass buffering and send character directly
     _pUart->UART_THR = uc_data;
  }
  return true;
}

void UARTClass::IrqHandler( void )
{
  uint32_t status = _pUart->UART_SR;

  // Did we receive data?
  if ((status & UART_SR_RXRDY) == UART_SR_RXRDY)
  {
    // Note: do not use the interrupt disabling/enabling 'atomic' methods but use the non-atomic core methods instead.
    if (! _rx_buffer->na__store_char(_pUart->UART_RHR))
    {
      setOverflowed();
    }
  }

  // Do we need to keep sending data?
  if ((status & UART_SR_TXRDY) == UART_SR_TXRDY) 
  {
	int c = _tx_buffer->na__read_char();
    if (c >= 0 /* valid char, -1 means buffer was empty */) {
      _pUart->UART_THR = c;
    }
    else
    {
      // Mask off transmit interrupt so we don't get it anymore
      _pUart->UART_IDR = UART_IDR_TXRDY;
    }
  }

  // Acknowledge errors
  uint8_t errors = status & (UART_SR_OVRE | UART_SR_FRAME | UART_SR_PARE); // bits 7, 6 and 5 so it fits into an uint8_t
  if (errors)
  {
    // error reporting outside ISR:
	setUARTstatusBits(errors);
	
	// reset status bits PARE, FRAME and OVRE
    _pUart->UART_CR |= UART_CR_RSTSTA;
  }
}


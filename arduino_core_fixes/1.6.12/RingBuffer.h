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

#ifndef _RING_BUFFER_
#define _RING_BUFFER_

#include <stdint.h>
#include <string.h>			// memset, ...
#include <sam3.h>           // __disable_irq() et al for Cortex M3 (SAM3 series)

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer, in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.

#define SERIAL_BUFFER_SIZE_DEFAULT   128 // SERIAL_BUFFER_SIZE

class RingBuffer
{
  protected:
    volatile int16_t _iHead{0};
    volatile int16_t _iTail{0};
	
  public:
    virtual uint16_t size() const = 0;
	// return (index % size()) :: this method helps prevent using a costly DIV/MOD op 
	// as the virtual methods prevent the compiler from optimizing the classic indexing
	// logic.
    virtual uint16_t wrapIndex(uint16_t index) const = 0;
	virtual volatile uint8_t *buffer() = 0;

  public:
	RingBuffer() {}

  // protected:
  public:			
    // the non-atomic methods will be useful for compound atomic operations and in interrupt handlers where the atomicity is implied.
	
	bool na__store_char( uint8_t c ) {
	  auto i = (_iHead + 1) % size();

	  // if we should be storing the received character into the location
	  // just before the tail (meaning that the head would advance to the
	  // current location of the tail), we're about to overflow the buffer
	  // and so we don't write the character or advance the head.
	  if ( i != _iTail ) {
	    buffer()[_iHead] = c;
	    _iHead = i;
		return true;
	  }
	  return false;
	}

	int na__available( void ) {
	  return wrapIndex(size() + _iHead - _iTail);
	}

	int na__availableForStore( void ) {
	  return wrapIndex(size() + _iTail - _iHead - 1);
	}

	int na__peek_char( void )
	{
	  // if the head isn't ahead of the tail, we don't have any characters
	  if ( _iHead == _iTail )
	    return -1;

	  return uint32_t(buffer()[_iTail]);
	}

	int na__read_char( void )
	{
	  // if the head isn't ahead of the tail, we don't have any characters
	  if ( _iHead == _iTail )
	    return -1;

	  uint8_t uc = buffer()[_iTail];
	  _iTail = wrapIndex(_iTail + 1);
	  return uint32_t(uc);
	}

	void na__drop( void )
	{
	  // clear the buffer i.e. drop all buffered output!
	  _iTail = _iHead;
	}

	bool na__isFlushed( void )
	{
	  // if the head isn't ahead of the tail, we don't have any characters
	  return _iTail == _iHead;
	}
	
  public:
	bool store_char( uint8_t c ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__store_char(c);
      __enable_irq();
	  return rv;
	}

	int available( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__available();
      __enable_irq();
	  return rv;
	}

	int availableForStore( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__availableForStore();
      __enable_irq();
	  return rv;
	}

	int peek_char( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__peek_char();
      __enable_irq();
	  return rv;
	}

	int read_char( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__read_char();
      __enable_irq();
	  return rv;
	}

	void flush( void ) {
	  while (_iHead != _iTail)
	    ; // Spin locks: wait for transmit data to be sent
	}

	void drop( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      na__drop();
      __enable_irq();
	}

	void reset( void )
	{
	  _iTail = 0;
	  _iHead = 0;
	}

	bool isFlushed( void ) {
	  // make it an atomic (non-interruptable) operation:
      __disable_irq();
      auto rv = na__isFlushed();
      __enable_irq();
	  return rv;
	}
};

template <uint16_t RB_BUFFER_SIZE = SERIAL_BUFFER_SIZE_DEFAULT>
class SizedRingBuffer final : public RingBuffer
{
  protected:
    volatile uint8_t _aucBuffer[RB_BUFFER_SIZE];
	
  public:
    virtual uint16_t size() const override {
	  return RB_BUFFER_SIZE;
	}

	virtual volatile uint8_t *buffer() override {
	  return _aucBuffer;
	}

    virtual uint16_t wrapIndex(uint16_t index) const override {
	  return index % RB_BUFFER_SIZE;
	}

  public:
	SizedRingBuffer() : RingBuffer() {
	  memset((void *)_aucBuffer, 0, sizeof(_aucBuffer));
	}
};

using SmallRingBuffer = SizedRingBuffer<SERIAL_BUFFER_SIZE_DEFAULT>;
using LargeRingBuffer = SizedRingBuffer<1024>;
using TinyRingBuffer  = SizedRingBuffer<64>;

#endif /* _RING_BUFFER_ */

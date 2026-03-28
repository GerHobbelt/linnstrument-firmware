/*********************************** ls_leds: LinnStrument LEDS ***********************************
Copyright 2023 Roger Linn Design (https://www.rogerlinndesign.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***************************************************************************************************
These functions handle the low-level communication with LinnStrument's 208 RGB LEDs.
**************************************************************************************************/

#include "ls_compiler_tweaks.h"

volatile uint32_t TC_hitcount[9] = { 0 };

void TC0_Handler(void) {
  // You must do TC_GetStatus to "accept" interrupt
  // As parameters use the first two parameters used in startTimer (TCx, ch) in this case.
  TC_GetStatus(TC0, 0);

  TC_hitcount[0]++;
}
void TC1_Handler(void) {
  TC_GetStatus(TC0, 1);

    TC_hitcount[1]++;
}
void TC2_Handler(void) {  
  TC_GetStatus(TC0, 2);

  TC_hitcount[2]++;
}
void TC3_Handler(void) {
  TC_GetStatus(TC1, 0);

    TC_hitcount[3]++;
}
void TC4_Handler(void) {
  TC_GetStatus(TC1, 1);

    TC_hitcount[4]++;
}
void TC5_Handler(void) {
  TC_GetStatus(TC1, 2);

    TC_hitcount[5]++;
}
void TC6_Handler(void) {
  TC_GetStatus(TC2, 0);

    TC_hitcount[6]++;
}
void TC7_Handler(void) {
  TC_GetStatus(TC2, 1);

    TC_hitcount[7]++;
}
void TC8_Handler(void) {
  TC_GetStatus(TC2, 2);

    TC_hitcount[8]++;
}

// https://forum.arduino.cc/t/timer-interrupts-on-due/127643/19

/*
   36.:: Timer Counter (TC)
   36.1:: Description

   A Timer Counter (TC) module includes three identical TC channels. The number of implemented TC modules is
   device-specific.

   Each TC channel can be independently programmed to perform a wide range of functions including frequency
   measurement, event counting, interval measurement, pulse generation, delay timing and pulse width modulation.
   Each channel has three external clock inputs, five internal clock inputs and two multi-purpose input/output signals
   which can be configured by the user. Each channel drives an internal interrupt signal which can be programmed to
   generate processor interrupts.

   The TC embeds a quadrature decoder (QDEC) connected in front of the timers and driven by TIOA0, TIOB0 and
   TIOB1 inputs. When enabled, the QDEC performs the input lines filtering, decoding of quadrature signals and
   connects to the timers/counters in order to read the position and speed of the motor through the user interface.
   The TC block has two global registers which act upon all TC channels:
   
   - Block Control Register (TC_BCR)—allows channels to be started simultaneously with the same instruction
   
   - Block Mode Register (TC_BMR)—defines the external clock inputs for each channel, allowing them to be
     chained

   36.5.2:: Power Management

   The TC is clocked through the Power Management Controller (PMC), thus the programmer must first configure the
   PMC to enable the Timer Counter clock.   
*/

// Black magic
void startHFLEDpaintTimer(uint32_t frequency) {
  auto tc = TC1;
  tc->TC_WPMR = (0x54494D << 8) | 0; // WPEN=0

#if 0
		if (!TCChanEnabled[interfaceID]) {
			pmc_enable_periph_clk(ID_TC3);  // TC1, channel 0 --> #TC3 !
			TC_Configure(chTC, chNo,
				TC_CMR_TCCLKS_TIMER_CLOCK1 |
				TC_CMR_WAVE |         // Waveform mode
				TC_CMR_WAVSEL_UP_RC | // Counter running up and reset when equals to RC
				TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
				TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
				TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);
			TC_SetRC(chTC, chNo, TC);
		}
#endif  
  pmc_set_writeprotect(false);
  /*
     DIV must not be changed while peripheral is in use or when the peripheral clock is enabled.To change the clock
     division factor (DIV) of a peripheral, its clock must first be disabled by writing either EN to 0 for the corresponding
     PID (DIV must be kept the same if this method is used), or writing to PMC_PCDR register. Then a second write
     must be performed into PMC_PCR with the new value of DIV and a third write must be performed to enable the
     peripheral clock (either by using PMC_PCR or PMC_PCER register).
  */
  pmc_disable_periph_clk(ID_TC3);  // TC1, channel 0 --> #TC3 !
  // PMC_PCR / PMC_PCDR / PMC_PCER
  REG_PMC_PCR = PMC_PCR_EN | PMC_PCR_DIV_PERIPH_DIV_MCK | PMC_PCR_CMD | PMC_PCR_PID(ID_TC3);
  pmc_enable_periph_clk(ID_TC3);

  constexpr const auto channel = 0;
#if 0
  TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
  uint32_t rc = VARIANT_MCK/128/frequency; //128 because we selected TIMER_CLOCK4 above
#else
  TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1 |
        TC_CMR_EEVT_XC0 |      // see note about TIOB vs no IRQ further below: EEVT must not be kept 0 if we want RB Compare IRQ.
				TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_ASWTRG_SET |
				TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_SET | TC_CMR_BSWTRG_SET);
  uint32_t rc = VARIANT_MCK/2/frequency; //2 because we selected TIMER_CLOCK1 above; see also "Table 36-1. Timer Counter Clock Assignment"
#endif
  TC_SetRA(tc, channel, rc/4); //50% high, 50% low
  TC_SetRB(tc, channel, rc/2);
  TC_SetRC(tc, channel, rc);

  /*
     Note: we were observing 2 interrupts per cycle: RA and RC compare, but not RB compare.
     This turned out to be due to us having set (default!) EEVT to TIOB; see the datasheet:

     The EEVT parameter in TC_CMR selects the external trigger. The EEVTEDG parameter defines the trigger edge
     for each of the possible external triggers (rising, falling or both). If EEVTEDG is cleared (none), no external event
     is defined.
     If TIOB is defined as an external event signal (EEVT = 0), TIOB is no longer used as an output and the compare
     register B is not used to generate waveforms and subsequently no IRQs. In this case the TC channel can only
     generate a waveform on TIOA.

     If TIOB is chosen as the external event signal, it is configured as an input and no longer generates waveforms and
     subsequently no IRQs.
  */
  tc->TC_CHANNEL[channel].TC_IER = TC_IER_CPCS | TC_IER_CPAS | TC_IER_CPBS;
  tc->TC_CHANNEL[channel].TC_IDR = 0; // ~TC_IER_CPCS;
  tc->TC_CHANNEL[channel].TC_IMR = 0; // enable all interrupts
  NVIC_EnableIRQ(TC3_IRQn);  // TC1, channel 0 --> #TC3 !

  TC_Start(tc, channel);

  //tc->TC_WPMR = (0x54494D << 8) | 1; // WPEN=1
}

void stopHFLEDpaintTimer() {
  auto tc = TC1;
  tc->TC_WPMR = (0x54494D << 8) | 0; // WPEN=0

  constexpr const auto channel = 0;
  
  TC_Stop(tc, channel);

  //tc->TC_WPMR = (0x54494D << 8) | 1; // WPEN=1
}

  // Start timer. Parameters are:

  // TC1 : timer counter. Can be TC0, TC1 or TC2
  // 0   : channel. Can be 0, 1 or 2
  // TC3_IRQn: irq number. See table.
  // 40  : frequency (in Hz)
  // The interrupt service routine is TC3_Handler. See table.

  //startTimer(TC1, 0, TC3_IRQn, 40);

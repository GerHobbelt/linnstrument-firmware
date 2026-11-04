
# Architectural Considerations


AI Overview

The SAM3X8E features an undocumented hardware conflict between **ADC Channel 15** (the internal temperature sensor) and **PIO Port B27** (which corresponds to Digital Pin 13 on the Arduino Due). 

When you activate AD15, the microcontroller usurps the internal bus line used by PB27, effectively forcing the PIO port to switch to an input state and disrupting any digital output (or LED blinking) you have assigned to Pin 13. 

## Workarounds to Resolve the Conflict

**1. Fast Toggle Method (Disable ADC after reading)**  
Instead of leaving the ADC internal temperature channel enabled in the background, configure the ADC, start the conversion, read Channel 15, and then disable the channel again to restore the port to its normal output state. 

cpp

```
int readInternalTemp() {
    // 1. Enable Channel 15 (Temperature Sensor)
    ADC->ADC_CHER = ADC_CHER_CH15;
    
    // 2. Start ADC conversion
    ADC->ADC_CR = ADC_CR_START;
    
    // 3. Wait for End of Conversion
    while ((ADC->ADC_ISR & ADC_ISR_EOC15) == 0);
    
    // 4. Read the temperature data
    int tempValue = ADC->ADC_CDR[15];
    
    // 5. CRITICAL: Disable Channel 15 to restore PB27 (Pin 13)
    ADC->ADC_CHDR = ADC_CHDR_CH15;
    
    return tempValue;
}
```

Use code with caution.

**2. Remap ADC Channels**  
Alternatively, you can reconfigure the ADC's "User Sequence" to ensure Channel 15 is not assigned to the default array index that overlaps with PB27's logic in the analog sequence registers. 

For more details on bare-metal register manipulation or the user sequence workaround, you can review the community breakdown on the [Arduino Forum](https://forum.arduino.cc/t/reading-the-temperature-sensor-inside-arduino-due/180830). 


---

## References 


- [due/180830](https://forum.arduino.cc/t/reading-the-temperature-sensor-inside-arduino-due/180830)
    
    Reading the temperature sensor inside Arduino Due
    
    {"passageText":"ADC15 and PB27 Conflict: Enabling ADC channel 15 (for the internal temperature sensor) on the SAM3X8E microcontrol...
    
    ![[IMG-20260528232357505.png]]
    
    forum.arduino.cc
    
- [ST/297793](https://community.st.com/t5/stm32-mcus-products/can-t-get-internal-temperature-sensor-via-adc/td-p/297793)
    
    Solved: Can't get internal temperature sensor via ADC
    
    8 Sept 2019 — To actually perform conversions, you need to start the ADC by setting ADCx_CR.ADSTART=1, and then wait for the conversion to end b...
    
    ![[IMG-20260528232357511.png]]
    
    STMicroelectronics Community
    

AI can make mistakes, so double-check responses



[due/18030](https://forum.arduino.cc/t/reading-the-temperature-sensor-inside-arduino-due/180830)

30 Aug 2013 — Despite the sensor it is not accurate (+-15%) it could be use as a warning if the _SAM3X8E_ gets low-temps (~ -40 C) or over-temps (~ +85 C).[Read more](https://forum.arduino.cc/t/reading-the-temperature-sensor-inside-arduino-due/180830#:~:text=Despite%20the%20sensor%20it%20is,or%20over%2Dtemps%20\(~%20%2B85%20C\).)

[_Sam3x8e temperature_ - Due - Arduino Forum](https://forum.arduino.cc/t/sam3x8e-temperature/149960)

11 Mar 2013

[Unable to serial print when using _ADC_ - Arduino Forum](https://forum.arduino.cc/t/unable-to-serial-print-when-using-adc/1122225)

2 May 2023


### ADC internal temp reading + ADC reading

  

![[IMG-20260528232357893.png]]

Raspberry Pi Forums

https://forums.raspberrypi.com › ... › General


](https://forums.raspberrypi.com/viewtopic.php?t=303013)

7 Feb 2021 — Hello I have a weird behaviour of the _pico_. When I _read_ the _internal temperature_ on _ADC_(4), no problem, I have the room _temperature_ using ...[Read more](https://forums.raspberrypi.com/viewtopic.php?t=303013#:~:text=Hello,the%20formula%20in%20the%20doc.)

Missing: ~~b27~~ ‎| Show results with: [b27](https://www.google.com/search?num=10&sca_esv=8019248285bfd55b&sxsrf=ANbL-n6Eys_qMee9Dw7IMOPKcRzevs9clg:1780003238687&q=sam3x8e+reading+internal+temperature+adc+resets+pio+port+%22b27%22+to+input&sa=X&ved=2ahUKEwiLvfbl9NyUAxXWlJUCHWcsBvAQ5t4CegQINhAB)

[

### SAM3X8E (Arduino Due) Pin IO registers

  

![[IMG-20260528232358010.png]]

Arduino Stack Exchange

https://arduino.stackexchange.com › questions › sam3x...







](https://arduino.stackexchange.com/questions/9117/sam3x8e-arduino-due-pin-io-registers)

8 Mar 2015 — How do the IO registers of Arduino Due work? On Arduino Uno just set DDRx, then PINx to read, PORTx to write, I'd like to do the same thing with an Arduino ...

[4 answers](https://arduino.stackexchange.com/a/12584) · Top answer: If you have a read of section 31 of the Datasheet, available from here, things may come ...

Missing: ~~temperature~~ ‎~~b27~~

[

### Solved: Why am I getting wrong internal temperature values...

  

![[IMG-20260528232358102.png]]

STMicroelectronics Community

https://community.st.com › stm32-mcus-products › td-p







](https://community.st.com/t5/stm32-mcus-products/why-am-i-getting-wrong-internal-temperature-values/td-p/270192)

1 Feb 2024 — The _internal temperature_ is totally off. It measures around 95 °C, but the MCU clearly feels less than human _body temperature_.[Read more](https://community.st.com/t5/stm32-mcus-products/why-am-i-getting-wrong-internal-temperature-values/td-p/270192#:~:text=But%20the%20internal%20temperature%20is,less%20than%20human%20body%20temperature.)

[STM32F103C8 _ADC internal temperature read_ way ...](https://community.st.com/t5/stm32-mcus-products/stm32f103c8-adc-internal-temperature-read-way-too-high/td-p/191892)

6 answers

25 Sept 2023

[STM32 _ADC_ Error - STMicroelectronics Community](https://community.st.com/t5/stm32-mcus-products/stm32-adc-error/td-p/784916)

2 answers

19 Mar 2025


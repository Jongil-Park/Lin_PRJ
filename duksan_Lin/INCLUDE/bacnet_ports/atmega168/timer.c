/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#include <stdint.h>
#include "hardware.h"

/* This module is a 1 millisecond timer */

/* Prescaling: 1, 8, 64, 256, 1024 */
#define TIMER_PRESCALER 64
/* Count: Timer0 counts up to 0xFF and then signals overflow */
#define TIMER_TICKS (F_CPU/TIMER_PRESCALER/1000)
#if (TIMER_TICKS > 0xFF)
#error Timer Prescaler value is too small
#endif
#define TIMER_COUNT (0xFF-TIMER_TICKS)
/* Global variable millisecond timer - used by main.c for timers task */
volatile uint8_t Timer_Milliseconds = 0;
/* MS/TP Silence Timer */
static volatile uint16_t SilenceTime;

/* Configure the Timer */
void Timer_Initialize(
    void)
{
    /* Normal Operation */
    TCCR1A = 0;
    /* CSn2 CSn1 CSn0 Description
       ---- ---- ---- -----------
       0    0    0  No Clock Source
       0    0    1  No prescaling
       0    1    0  CLKio/8
       0    1    1  CLKio/64
       1    0    0  CLKio/256
       1    0    1  CLKio/1024
       1    1    0  Falling Edge of T0 (external)
       1    1    1  Rising Edge of T0 (external)
     */
    TCCR0B = _BV(CS01) | _BV(CS00);
    /* Clear any TOV1 Flag set when the timer overflowed */
    BIT_CLEAR(TIFR0, TOV0);
    /* Initial value */
    TCNT0 = TIMER_COUNT;
    /* Enable the overflow interrupt */
    BIT_SET(TIMSK0, TOIE0);
    /* Clear the Power Reduction Timer/Counter0 */
    BIT_CLEAR(PRR, PRTIM0);
}

/* Timer interupt */
/* note: Global interupts must be enabled - sei() */
/* Timer Overflowed!  Increment the time. */
ISR(TIMER0_OVF_vect)
{
    /* Set the counter for the next interrupt */
    TCNT0 = TIMER_COUNT;
    /* Overflow Flag is automatically cleared */
    /* Update the global timer */
    if (Timer_Milliseconds < 0xFF)
        Timer_Milliseconds++;
    if (SilenceTime < 0xFFFF)
        SilenceTime++;
}

/* Public access to the Silence Timer */
uint16_t Timer_Silence(
    void)
{
    uint16_t timer;
    uint8_t sreg;

    sreg = SREG;
    __disable_interrupt();
    timer = SilenceTime;
    SREG = sreg;

    return timer;
}

/* Public reset of the Silence Timer */
void Timer_Silence_Reset(
    void)
{
    uint8_t sreg;

    sreg = SREG;
    __disable_interrupt();
    SilenceTime = 0;
    SREG = sreg;
}

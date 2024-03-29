/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware.h"
#include "fifo.h"
#include "timer.h"
#include "led.h"
#include "nvdata.h"

/* baud rate */
static uint32_t Baud_Rate = 9600;

/* The minimum time after the end of the stop bit of the final octet of a */
/* received frame before a node may enable its EIA-485 driver: 40 bit times. */
/* At 9600 baud, 40 bit times would be about 4.166 milliseconds */
/* At 19200 baud, 40 bit times would be about 2.083 milliseconds */
/* At 38400 baud, 40 bit times would be about 1.041 milliseconds */
/* At 57600 baud, 40 bit times would be about 0.694 milliseconds */
/* At 76800 baud, 40 bit times would be about 0.520 milliseconds */
/* At 115200 baud, 40 bit times would be about 0.347 milliseconds */
/* 40 bits is 4 octets including a start and stop bit with each octet */
#define Tturnaround  (40UL)
/* turnaround_time_milliseconds = (Tturnaround*1000UL)/Baud_Rate; */

/* buffer for storing received bytes - size must be power of two */
static uint8_t Receive_Buffer_Data[128];
static FIFO_BUFFER Receive_Buffer;

static void rs485_rts_init(
    void)
{
    /* configure the port pin as an output */
    BIT_SET(DDRD, DDD4);
}

/* enable the transmit-enable line on the RS-485 transceiver */
void rs485_rts_enable(
    bool enable)
{
    if (enable) {
        BIT_SET(PORTD, PD4);
    } else {
        BIT_CLEAR(PORTD, PD4);
    }
}

static void rs485_receiver_enable(
    void)
{
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
}

void rs485_turnaround_delay(
    void)
{
    uint16_t turnaround_time;

    /* delay after reception before trasmitting - per MS/TP spec */
    /* wait a minimum  40 bit times since reception */
    /* at least 1 ms for errors: rounding, clock tick */
    turnaround_time = 1 + ((Tturnaround * 1000UL) / Baud_Rate);
    while (!timer_elapsed_milliseconds(TIMER_SILENCE, turnaround_time)) {
        /* do nothing - wait for timer to increment */
    };
}

ISR(USART0_RX_vect)
{
    uint8_t data_byte;

    if (BIT_CHECK(UCSR0A, RXC0)) {
        /* data is available */
        data_byte = UDR0;
        (void) FIFO_Put(&Receive_Buffer, data_byte);
        timer_reset(TIMER_SILENCE);
    }
}

bool rs485_byte_available(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        led_on(LED_1);
        *data_register = FIFO_Get(&Receive_Buffer);
        data_available = true;
        led_off_delay(LED_1, 10);
    }

    return data_available;
}

bool rs485_receive_error(
    void)
{
    return false;
}

void rs485_bytes_send(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    led_on(LED_2);
    while (!BIT_CHECK(UCSR0A, UDRE0)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    while (nbytes) {
        /* Send the data byte */
        UDR0 = *buffer;
        while (!BIT_CHECK(UCSR0A, UDRE0)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        buffer++;
        nbytes--;
    }
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR0A, TXC0)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR0A, TXC0);
    timer_reset(TIMER_SILENCE);
    led_off_delay(LED_2, 1);

    return;
}

uint32_t rs485_baud_rate(
    void)
{
    return Baud_Rate;
}

static void rs485_baud_rate_configure(
    void)
{
    /* 2x speed mode */
    BIT_SET(UCSR0A, U2X0);
    /* configure baud rate */
    UBRR0 = (F_CPU / (8UL * Baud_Rate)) - 1;
}

bool rs485_baud_rate_set(
    uint32_t baud)
{
    bool valid = true;
    uint8_t baud_k = 0;

    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            Baud_Rate = baud;
            rs485_baud_rate_configure();
            /* store the baud rate */
            baud_k = baud / 1000;
            eeprom_bytes_write(NV_EEPROM_BAUD_K, &baud_k, 1);
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

static void rs485_usart_init(
    void)
{
    /* enable Transmit and Receive */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);
    /* Set USART Control and Status Register n C */
    /* Asynchronous USART 8-bit data, No parity, 1 stop */
    /* Set USART Mode Select: UMSELn1 UMSELn0 = 00 for Asynchronous USART */
    /* Set Parity Mode:  UPMn1 UPMn0 = 00 for Parity Disabled */
    /* Set Stop Bit Select: USBSn = 0 for 1 stop bit */
    /* Set Character Size: UCSZn2 UCSZn1 UCSZn0 = 011 for 8-bit */
    /* Clock Polarity: UCPOLn = 0 when asynchronous mode is used. */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    /* Clear Power Reduction */
    BIT_CLEAR(PRR, PRUSART0);
}

static void rs485_init_nvdata(
    void)
{
    uint8_t baud_k = 9; /* from EEPROM value */

    eeprom_bytes_read(NV_EEPROM_BAUD_K, &baud_k, 1);
    switch (baud_k) {
        case 9:
            Baud_Rate = 9600;
            break;
        case 19:
            Baud_Rate = 19200;
            break;
        case 38:
            Baud_Rate = 38400;
            break;
        case 57:
            Baud_Rate = 57600;
            break;
        case 76:
            Baud_Rate = 76800;
            break;
        case 115:
            Baud_Rate = 115200;
            break;
        default:
            /* not configured yet */
            Baud_Rate = 38400;
            baud_k = 38400 / 1000;
            eeprom_bytes_write(NV_EEPROM_BAUD_K, &baud_k, 1);
            break;
    }
    rs485_baud_rate_configure();
}

void rs485_init(
    void)
{
    FIFO_Init(&Receive_Buffer, &Receive_Buffer_Data[0],
        (unsigned) sizeof(Receive_Buffer_Data));
    rs485_rts_init();
    rs485_usart_init();
    rs485_init_nvdata();
    rs485_receiver_enable();
    rs485_rts_enable(false);
}

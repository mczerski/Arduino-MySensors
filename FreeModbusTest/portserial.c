/*
 * FreeModbus Libary: ATMega168 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *   - Initial version and ATmega168 support
 * Modfications Copyright (C) 2006 Tran Minh Hoang:
 *   - ATmega8, ATmega16, ATmega32 support
 *   - RS485 support for DS75176
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#define UART_BAUD_RATE          9600
#define UART_BAUD_CALC(UART_BAUD_RATE,FOSC) \
    ((F_CPU + UART_BAUD_RATE * 8UL) / (UART_BAUD_RATE * 16UL) - 1UL)

//#define UART_UCSRB  UCSR0B

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
#ifdef RTS_ENABLE
    UCSR0B |= _BV( TXEN0 ) | _BV(TXCIE0);
#else
    UCSR0B |= _BV(TXEN0);
#endif

    if( xRxEnable )
    {
        UCSR0B |= _BV( RXEN0 ) | _BV(RXCIE0);
    }
    else
    {
        UCSR0B &= ~( _BV( RXEN0 ) | _BV(RXCIE0));
    }

    if( xTxEnable )
    {
        UCSR0B |= _BV( TXEN0 ) | _BV(UDRIE0);
#ifdef RTS_ENABLE
        RTS_HIGH;
#endif
    }
    else
    {
        UCSR0B &= ~(_BV(UDRIE0));
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    UCHAR ucUCSRC = 0;

    /* prevent compiler warning. */
    (void)ucPORT;
	
    UBRR0H = UART_BAUD_CALC( ulBaudRate, F_CPU ) >> 8;;
    UBRR0L = UART_BAUD_CALC( ulBaudRate, F_CPU );

#if USE_2X
    UCSR0A |= (1<<U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    switch ( eParity )
    {
        case MB_PAR_EVEN:
            ucUCSRC |= _BV(UPM01);
            break;
        case MB_PAR_ODD:
            ucUCSRC |= _BV( UPM01 ) | _BV(UPM00);
            break;
        case MB_PAR_NONE:
            break;
    }

    switch ( ucDataBits )
    {
        case 8:
            ucUCSRC |= _BV( UCSZ00 ) | _BV(UCSZ01);
            break;
        case 7:
            ucUCSRC |= _BV(UCSZ01);
            break;
    }

    UCSR0C |= ucUCSRC;

    vMBPortSerialEnable( FALSE, FALSE );

#ifdef RTS_ENABLE
    RTS_INIT;
#endif
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    UDR0 = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = UDR0;
    return TRUE;
}

ISR( USART_UDRE_vect )
{
    pxMBFrameCBTransmitterEmpty(  );
}

ISR( USART_RX_vect )
{
    pxMBFrameCBByteReceived(  );
}

#ifdef RTS_ENABLE
ISR( USART_TX_vect )
{
    RTS_LOW;
}
#endif


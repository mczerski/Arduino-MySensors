#ifndef _MB_WITH_WDT_H
#define _MB_WITH_WDT_H

#include "mb.h"
#include <avr/wdt.h>

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

eMBErrorCode eMBInitWithWDT(eMBMode eMode, UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity, uint8_t wdt_value);
eMBErrorCode eMBPollWithWDT();

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif

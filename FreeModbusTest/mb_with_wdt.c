#include "mb_with_wdt.h"

eMBErrorCode eMBInitWithWDT(eMBMode eMode, UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity, const uint8_t wdt_value)
{
    wdt_enable(wdt_value);
    return eMBInit(eMode, ucSlaveAddress, ucPort, ulBaudRate, eParity);
}

eMBErrorCode eMBPollWithWDT()
{
    wdt_reset();
    eMBPoll();
}

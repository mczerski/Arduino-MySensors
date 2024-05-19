#include "mb_with_wdt.h"
#include "SPIFlash.h"

SPIFlash flash(8, 0x1F65);

MBFile fileTable[] = {
    {.fileNumber = 1, .fileSize = 10, .fileOffset = 0},
    {.fileNumber = 2, .fileSize = 32*1024 - 10, .fileOffset = 10},
    {.fileNumber = 0, .fileSize = 0, .fileOffset = 0}
};

eMBErrorCode eMBInitWithWDT(eMBMode eMode, UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity, const uint8_t wdt_value)
{
    wdt_enable(wdt_value);
    if (!flash.initialize())
        return MB_EIO;
    return eMBInit(eMode, ucSlaveAddress, ucPort, ulBaudRate, eParity);
}

eMBErrorCode eMBPollWithWDT()
{
    wdt_reset();
    eMBPoll();
}

eMBErrorCode eMBRegFileCB(UCHAR * pucFileBuffer, USHORT usFileNumber, USHORT usRecordNumber, USHORT usRecordLength, eMBRegisterMode eMode) {
  if (eMode == MB_REG_WRITE) {
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            flash.writeBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            while (flash.busy());
            return MB_ENOERR;
        }
    }
  }
  else if (eMode == MB_REG_READ) {
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            flash.readBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            return MB_ENOERR;
        }
    }
  }
  return MB_ENOREG;
}

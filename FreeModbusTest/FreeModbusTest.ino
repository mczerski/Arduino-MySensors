#include "mb_with_wdt.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 6

uint16_t holding[100] = {0};

void setup() {
   const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
   eMBErrorCode eStatus;

   eStatus = eMBInitWithWDT(MB_RTU, 1, 0, 500000, MB_PAR_NONE, WDTO_8S, VERSION_MAJOR, VERSION_MINOR);

   eStatus = eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);

  /* Enable the Modbus Protocol Stack. */
  eStatus = eMBEnable();

  holding[1] = VERSION_MAJOR << 8 + VERSION_MINOR;
}

void loop() {
  eMBPollWithWDT();
  if (holding[99]) {
    while (1);
  }
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  uint16_t i;
  if (usAddress + (usNRegs - 1) >= 100) return MB_ENOREG;

  for (i = 0; i < usNRegs; i++) {
    if (eMode == MB_REG_READ) {
      pucRegBuffer[i * 2 + 0] = (unsigned char) (holding[i + usAddress] >> 8);
      pucRegBuffer[i * 2 + 1] = (unsigned char) (holding[i + usAddress] & 0xFF);
    } else if (eMode == MB_REG_WRITE) {
      if (usAddress == 0) {
        return MB_ENOREG;
      }
      holding[i + usAddress] = (pucRegBuffer[i * 2 + 0] << 8) | pucRegBuffer[i * 2 + 1];
    }
  }
  return MB_ENOERR;
}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  return MB_ENOREG;
}

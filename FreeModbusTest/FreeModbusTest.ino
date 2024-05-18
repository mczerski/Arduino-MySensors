#include "mb.h"

void setup() {
   const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
   eMBErrorCode eStatus;

   eStatus = eMBInit(MB_RTU, 0x0A, 0, 38400, MB_PAR_NONE);

   eStatus = eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);

  /* Enable the Modbus Protocol Stack. */
  eStatus = eMBEnable();
}

void loop() {
  eMBPoll();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return MB_ENOREG;
}

uint16_t holding[100];

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
        uint16_t i;
        if (usAddress + (usNRegs - 1) >= 100) return MB_ENOREG;

        for (i = 0; i < usNRegs; i++) {

                if (eMode == MB_REG_READ) {

                        pucRegBuffer[i * 2 + 0] = (unsigned char) (holding[i + usAddress] >> 8);
                        pucRegBuffer[i * 2 + 1] = (unsigned char) (holding[i + usAddress] & 0xFF);

                } else if (eMode == MB_REG_WRITE) {

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

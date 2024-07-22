#include "mb_with_wdt.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
#define MOTION_PIN 3

uint16_t holding[10] = {0};

void setup() {
  pinMode(MOTION_PIN, INPUT_PULLUP);
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, VERSION_MAJOR, VERSION_MINOR);
  holding[0] = VERSION_MAJOR << 8 + VERSION_MINOR;
}

void loop() {
  holding[1] = digitalRead(MOTION_PIN);
  eMBPollWithWDT();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  if (usAddress + (usNRegs - 1) >= (sizeof(holding) / sizeof(holding[0]))) return MB_ENOREG;

  for (uint16_t i = 0; i < usNRegs; i++) {
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

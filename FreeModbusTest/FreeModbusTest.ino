#include "mb_with_wdt.h"
#include <Dimmer.h>
#include <BounceSwitch.h>
#include <SceneController.h>

#define VERSION_MAJOR 2
#define VERSION_MINOR 2

using namespace mys_toolkit;

BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
CwWwDimmer dimmer(5, 6, false, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene;

void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, VERSION_MAJOR, VERSION_MINOR);
  dimmer.begin();
}

void loop() {
  dimmer.update(sw1.update());
  scene.update(sw2.update());
  eMBPollWithWDT();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  if (usAddress >= 0 or usAddress + usNRegs - 1 <= 2) {
    for (USHORT i = 0; i < usNRegs; i++) {
      switch (i) {
        case 0:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = dimmer.getLevel();
           break;
        case 1:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene.getScene0Counter();
           break;
        case 2:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene.getScene1Counter();
           break;
        default:
           break;
      }
    }
    return MB_ENOERR;
  }
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  if (usAddress >= 0 or usAddress + usNRegs - 1 <= 0) {
    for (USHORT i = 0; i < usNRegs; i++) {
      if (eMode == MB_REG_WRITE) {
        switch (i) {
          case 0:
            dimmer.request(pucRegBuffer[i * 2 + 1]);
            break;
          default:
            break;            
        }
        return MB_ENOERR;
      }
    }
  }
  return MB_ENOREG;
}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  return MB_ENOREG;
}

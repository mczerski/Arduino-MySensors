#include "mb_with_wdt.h"
#include <Dimmer.h>
#include <BounceSwitch.h>
#include <SceneController.h>

#define VERSION_MAJOR 2
#define VERSION_MINOR 4

#define LIVINGROOM_LED

using namespace mys_toolkit;

#ifdef LIVINGROOM_LED
#define DIMMER1
#define SCENE2
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
CwWwDimmer dimmer1(5, 6, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef LIVINGROOM_SPOT
#define DIMMER1
#define SCENE2
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
SimpleDimmer dimmer1(5, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef KITCHEN_SPOT
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
SimpleDimmer dimmer1(5, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, VERSION_MAJOR, VERSION_MINOR);
  #ifdef DIMMER1
  dimmer1.begin();
  #endif
  #ifdef DIMMER2
  dimmer2.begin();
  #endif
}

void loop() {
  #ifdef DIMMER1
  dimmer1.update(sw1.update());
  #endif
  #ifdef DIMMER2
  dimmer2.update(sw2.update());
  #endif
  #ifdef SCENE1
  scene1.update(sw1.update());
  #endif
  #ifdef SCENE2
  scene2.update(sw2.update());
  #endif
  #ifdef SCENE3
  scene3.update(sw3.update());
  #endif
  #ifdef SCENE4
  scene4.update(sw4.update());
  #endif
  eMBPollWithWDT();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  if (usAddress >= 1 and usAddress + usNRegs - 1 <= 10) {
    for (USHORT i = 0; i < usNRegs; i++) {
      switch (usAddress + i) {
#ifdef DIMMER1
        case 1:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = dimmer1.getPercent();
           break;
#endif
#ifdef DIMMER2
        case 2:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = dimmer2.getPercent();
           break;
#endif
#ifdef SCENE1
        case 3:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene1.getScene0Counter();
           break;
        case 4:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene1.getScene1Counter();
           break;
#endif
#ifdef SCENE2
        case 5:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene2.getScene0Counter();
           break;
        case 6:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene2.getScene1Counter();
           break;
#endif
#ifdef SCENE3
        case 7:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene3.getScene0Counter();
           break;
        case 8:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene3.getScene1Counter();
           break;
#endif
#ifdef SCENE4
        case 9:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene4.getScene0Counter();
           break;
        case 10:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = scene4.getScene1Counter();
           break;
#endif
        default:
           break;
      }
    }
    return MB_ENOERR;
  }
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  if (usAddress >= 1 and usAddress + usNRegs - 1 <= 1) {
    for (USHORT i = 0; i < usNRegs; i++) {
      if (eMode == MB_REG_WRITE) {
        switch (usAddress + i) {
#ifdef DIMMER1
          case 1:
            if (pucRegBuffer[i * 2 + 1] == 255) {
              dimmer1.set(true);
            }
            else {
              dimmer1.setPercent(pucRegBuffer[i * 2 + 1]);
            }
            break;
#endif
#ifdef DIMMER2
          case 2:
            if (pucRegBuffer[i * 2 + 1] == 255) {
              dimmer2.set(true);
            }
            else {
              dimmer2.setPercent(pucRegBuffer[i * 2 + 1]);
            }
            break;
#endif
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

#include <mb_with_wdt.h>
#include <Dimmer.h>
#include <BounceSwitch.h>
#include <SceneController.h>

#define VERSION_MAJOR "2"
#define VERSION_MINOR "11"

#define KITCHEN_LED

using namespace mys_toolkit;

#ifdef TEST
#define SLAVE_ID 0
#define SLAVE_NAME "Test"
#define DIMMER1
#define SCENE2
#define FLASH_ID 0xC840
#define LED_PIN -1
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
SimpleDimmer dimmer1(A1, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef LIVINGROOM_LED
#define SLAVE_ID 1
#define SLAVE_NAME "Livingroom LED"
#define DIMMER1
#define SCENE2
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define PRESCALER DurationPrescaler::CLK_64
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
CwWwDimmer dimmer1(5, 6, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef LIVINGROOM_SPOT
#define SLAVE_ID 2
#define SLAVE_NAME "Livingroom Spotlight"
#define DIMMER1
#define SCENE2
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define PRESCALER DurationPrescaler::CLK
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
CwWwDimmer dimmer1(5, 6, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef KITCHEN_SPOT
#define SLAVE_ID 3
#define SLAVE_NAME "Kitchen Spotlight"
#define DIMMER1
#define SCENE2
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define PRESCALER DurationPrescaler::CLK
BounceSwitch sw1(3, Duration(50), true);
BounceSwitch sw2(4, Duration(50), true);
SimpleDimmer dimmer1(5, true, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene2;
#endif

#ifdef KITCHEN_LED
#include <APDS9930Switch.h>
#include <MiLightDimmer.h>
#define SLAVE_ID 5
#define SLAVE_NAME "Kitchen LED"
#define DIMMER1
#define DIMMER2
#define SCENE3
#define FLASH_ID 0xC840
#define LED_PIN A1
#define PRESCALER DurationPrescaler::CLK_64
#define APDS9930
#define MI_LIGHT
#define NRF24_CE_PIN 7
#define NRF24_CSN_PIN 6
MyAPDS9930 apds9930(2, 1);
RF24 nrf24Radio(NRF24_CE_PIN, NRF24_CSN_PIN);
PL1167_nRF24 pl1167(nrf24Radio);
APDS9930Switch sw1(apds9930, 0);
BounceSwitch sw2(3, Duration(50), true);
BounceSwitch sw3(4, Duration(50), true);
SimpleDimmer dimmer1(5, true, 10, {.slowDimming=1, .fullBrightness=1});
MiLightDimmer dimmer2(pl1167, 0xF2EA, 4, false, 10, {.slowDimming=1, .fullBrightness=1});
SceneController scene3;
#endif

const char additional_info[] = SLAVE_NAME " v" VERSION_MAJOR "." VERSION_MINOR;
SPIFlash flash(8, FLASH_ID);
struct InputRegisters_ {
  uint16_t dimmer1_state;
  uint16_t dimmer2_state;
  uint16_t scene1_0_counter;
  uint16_t scene1_1_counter;
  uint16_t scene2_0_counter;
  uint16_t scene2_1_counter;
  uint16_t scene3_0_counter;
  uint16_t scene3_1_counter;
  uint16_t scene4_0_counter;
  uint16_t scene4_1_counter;
  uint16_t apds0030_status;
  uint16_t milight_status;
};
typedef ModbusDataBlock<InputRegisters_> InputRegisters;
InputRegisters input_register = {0};

void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, SLAVE_ID, additional_info, sizeof(additional_info) - 1, &flash, LED_PIN);
#ifdef DIMMER1
  dimmer1.begin();
#endif
#ifdef DIMMER2
  dimmer2.begin();
#endif
#ifdef APDS9930
  apds9930.init();
#endif
  Duration::setPrescaler(PRESCALER);
}

void loop() {
#ifdef DIMMER1
  dimmer1.update(sw1.update());
  input_register.s.dimmer1_state = dimmer1.getPercent();
#endif
#ifdef DIMMER2
  dimmer2.update(sw2.update());
  input_register.s.dimmer2_state = dimmer2.getPercent();
#endif
#ifdef SCENE1
  scene1.update(sw1.update());
  input_register.s.scene1_0_counter = scene1.getScene0Counter();
  input_register.s.scene1_1_counter = scene1.getScene1Counter();
#endif
#ifdef SCENE2
  scene2.update(sw2.update());
  input_register.s.scene2_0_counter = scene2.getScene0Counter();
  input_register.s.scene2_1_counter = scene2.getScene1Counter();
#endif
#ifdef SCENE3
  scene3.update(sw3.update());
  input_register.s.scene3_0_counter = scene3.getScene0Counter();
  input_register.s.scene3_1_counter = scene3.getScene1Counter();
#endif
#ifdef SCENE4
  scene4.update(sw4.update());
  input_register.s.scene4_0_counter = scene4.getScene0Counter();
  input_register.s.scene4_1_counter = scene4.getScene1Counter();
#endif
#ifdef APDS9930
  apds9930.update();
  input_register.s.apds0030_status = (apds9930.getInt(0) << 8) + apds9930.getErrorCode(0);
#endif
#ifdef MI_LIGHT
  input_register.s.milight_status = dimmer2.getErrorCode();
#endif
  eMBPollWithWDT();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return eMBCopyToRegBuffer(input_register, pucRegBuffer, usAddress, usNRegs);
}

eMBErrorCode eMBRegHoldingCB(USHORT * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  uint8_t dimmer1_state = 0;
  uint8_t dimmer2_state = 0;
  if (usAddress >= 1 and usAddress + usNRegs - 1 <= 2) {
    for (USHORT i = 0; i < usNRegs; i++) {
      if (eMode == MB_REG_WRITE) {
        switch (usAddress + i) {
#ifdef DIMMER1
          case 1:
            dimmer1_state = __builtin_bswap16(pucRegBuffer[i]);
            if (dimmer1_state == 255) {
              dimmer1.set(true);
            }
            else {
              dimmer1.setPercent(dimmer1_state);
            }
            break;
#endif
#ifdef DIMMER2
          case 2:
            dimmer2_state = __builtin_bswap16(pucRegBuffer[i]);
            if (dimmer2_state == 255) {
              dimmer2.set(true);
            }
            else {
              dimmer2.setPercent(dimmer2_state);
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

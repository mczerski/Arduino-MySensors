#include <mb_with_wdt.h>
#include <BounceSwitch.h>
#include <Relay.h>
#include <BL0942.h>
#include <TempSensor.h>
#include <EEPROM.h>

#define VERSION_MAJOR "1"
#define VERSION_MINOR "2"

#define TEST

using namespace mys_toolkit;

#ifdef TEST
#define SLAVE_ID 0
#define SLAVE_NAME "Test"
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define STATUS_LED_PIN 6
#define CF_PIN 2
BounceSwitch sw1(3, Duration(50), false);
GPIORelay relay1(5, Duration(250));
#endif

const char additional_info[] = SLAVE_NAME " v" VERSION_MAJOR "." VERSION_MINOR;
SPIFlash flash(8, FLASH_ID);
BL0942 bl0942(Serial1);
TempSensor tempSensor;
union ConfData {
  struct S {
    float i_gain;
    float v_gain;
    float temp_gain;
    int16_t temp_offset;
  } s;
  uint16_t data[sizeof(S) / sizeof(uint16_t)];
} __attribute__((packed)) __attribute__((scalar_storage_order("little-endian")));
static constexpr size_t CONF_REGS = sizeof(ConfData) / sizeof(uint16_t);
ConfData conf;

void cf_interrupt() {
  relay1.set(false);
}

void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, SLAVE_ID, additional_info, sizeof(additional_info) - 1, &flash, LED_PIN);
  Serial1.begin(4800);
  bl0942.begin();
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(CF_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CF_PIN), cf_interrupt, RISING);
  tempSensor.begin();

  EEPROM.get(2, conf);
  bl0942.calibrate(conf.s.i_gain, conf.s.v_gain);
  tempSensor.calibrate(conf.s.temp_offset, conf.s.temp_gain);

  // make sure relay is off
  delay(500);
  relay1.set(true);
  delay(500);
  relay1.set(false);
}

void loop() {
  tempSensor.update();
  relay1.update(sw1.update());
  analogWrite(STATUS_LED_PIN, relay1.getState() ? 10 : 0);
  bl0942.update();
  eMBPollWithWDT();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  if (usAddress >= 1 and usAddress + usNRegs - 1 <= 14 + CONF_REGS - 1) {
    for (USHORT i = 0; i < usNRegs; i++) {
      switch (usAddress + i) {
        case 1:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = relay1.getState();
           break;
        case 2:
           pucRegBuffer[i * 2 + 0] = (bl0942.getIRms_mA() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getIRms_mA() >> 0) & 0xff;
           break;
        case 3:
           pucRegBuffer[i * 2 + 0] = (bl0942.getIRms_mA() >> 24) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getIRms_mA() >> 16) & 0xff;
           break;
        case 4:
           pucRegBuffer[i * 2 + 0] = (bl0942.getVRms_mV() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getVRms_mV() >> 0) & 0xff;
           break;
        case 5:
           pucRegBuffer[i * 2 + 0] = (bl0942.getVRms_mV() >> 24) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getVRms_mV() >> 16) & 0xff;
           break;
        case 6:
           pucRegBuffer[i * 2 + 0] = (bl0942.getPRms_mW() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getPRms_mW() >> 0) & 0xff;
           break;
        case 7:
           pucRegBuffer[i * 2 + 0] = (bl0942.getPRms_mW() >> 24) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getPRms_mW() >> 16) & 0xff;
           break;
        case 8:
           pucRegBuffer[i * 2 + 0] = (bl0942.getE_mWh() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getE_mWh() >> 0) & 0xff;
           break;
        case 9:
           pucRegBuffer[i * 2 + 0] = (bl0942.getE_mWh() >> 24) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getE_mWh() >> 16) & 0xff;
           break;
        case 10:
           pucRegBuffer[i * 2 + 0] = (bl0942.getIFastRmsTh_mA() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getIFastRmsTh_mA() >> 0) & 0xff;
           break;
        case 11:
           pucRegBuffer[i * 2 + 0] = (bl0942.getIFastRmsTh_mA() >> 24) & 0xff;
           pucRegBuffer[i * 2 + 1] = (bl0942.getIFastRmsTh_mA() >> 16) & 0xff;
           break;
        case 12:
           pucRegBuffer[i * 2 + 0] = digitalRead(CF_PIN);
           pucRegBuffer[i * 2 + 1] = bl0942.getCfOutput();
           break;
        case 13:
           pucRegBuffer[i * 2 + 0] = (tempSensor.getTemp() >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (tempSensor.getTemp() >> 0) & 0xff;
           break;
        case 14 ... 14 + CONF_REGS - 1:
           pucRegBuffer[i * 2 + 0] = (conf.data[usAddress + i - 14] >> 8) & 0xff;
           pucRegBuffer[i * 2 + 1] = (conf.data[usAddress + i - 14] >> 0) & 0xff;
           break;
        default:
           pucRegBuffer[i * 2 + 0] = 0;
           pucRegBuffer[i * 2 + 1] = 0;
           break;
      }
    }
    return MB_ENOERR;
  }
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  bool update_conf = false;
  if (usAddress >= 1 and usAddress + usNRegs - 1 <= 2 + CONF_REGS - 1) {
    for (USHORT i = 0; i < usNRegs; i++) {
      if (eMode == MB_REG_WRITE) {
        switch (usAddress + i) {
          case 1:
            if (pucRegBuffer[i * 2 + 1] and not digitalRead(CF_PIN)) {
              relay1.set(true);
            }
            else {
              relay1.set(false);
            }
            break;
          case 2 ... 2 + CONF_REGS - 1:
            conf.data[usAddress + i - 2] = (pucRegBuffer[i * 2 + 0] << 8) + pucRegBuffer[i * 2 + 1];
            update_conf = true;
            break;
          default:
            break;
        }
      }
    }
    if (update_conf) {
      EEPROM.put(2, conf);
      bl0942.calibrate(conf.s.i_gain, conf.s.v_gain);
      tempSensor.calibrate(conf.s.temp_offset, conf.s.temp_gain);
    }
    return MB_ENOERR;
  }
  return MB_ENOREG;
}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  return MB_ENOREG;
}

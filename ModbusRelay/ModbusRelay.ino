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

struct ConfData_ {
  float i_gain;
  float v_gain;
  float temp_gain;
  int16_t temp_offset;
};
typedef ModbusDataBlock<ConfData_> ConfData;
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
  struct InputRegisters_ {
    uint16_t relay_state;
    int32_t i_rms_mA;
    int32_t v_rms_mV;
    int32_t p_rms_mW;
    int32_t e_mWh;
    int32_t fast_i_rms_mA;
    int16_t temperature;
    ConfData_ conf_data;
  };
  typedef ModbusDataBlock<InputRegisters_> InputRegisters;
  InputRegisters input_register = {
    relay1.getState(),
    bl0942.getIRms_mA(),
    bl0942.getVRms_mV(),
    bl0942.getPRms_mW(),
    bl0942.getE_mWh(),
    bl0942.getIFastRmsTh_mA(),
    tempSensor.getTemp(),
    conf.s,
  };
  return eMBCopyToRegBuffer(input_register, pucRegBuffer, usAddress, usNRegs);
}

eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  bool update_conf = false;
  if (usAddress >= 1 and usAddress + usNRegs <= 1 + ConfData::mbLength) {
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
          case 2 ... 2 + ConfData::mbLength - 1:
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

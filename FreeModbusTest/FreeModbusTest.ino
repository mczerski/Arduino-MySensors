#include "mb_with_wdt.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
#define LED_PIN A1
#define MOTION_PIN 3

uint16_t holding[10] = {0};
unsigned long led_turn_off = 0;

void init_led() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  
}

void trigger_led() {
  led_turn_off = millis() + 200;
  digitalWrite(LED_PIN, LOW);
}

void update_led() {
  if (millis() >= led_turn_off) {
    digitalWrite(LED_PIN, HIGH);
  }
}

void error(int code) {
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    delay(code * 200);
    digitalWrite(LED_PIN, LOW);
    delay(code * 200);
  }
}

void setup() {
  init_led();
  pinMode(MOTION_PIN, INPUT_PULLUP);
  
  const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
  eMBErrorCode eStatus;

  eStatus = eMBInitWithWDT(MB_RTU, 1, 0, 500000, MB_PAR_NONE, WDTO_8S, VERSION_MAJOR, VERSION_MINOR);
  if (eStatus != MB_ENOERR) error(1);
  eStatus = eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);
  if (eStatus != MB_ENOERR) error(2);
  eStatus = eMBEnable();
  if (eStatus != MB_ENOERR) error(3);

  holding[0] = VERSION_MAJOR << 8 + VERSION_MINOR;
  trigger_led();
}

void loop() {
  holding[1] = digitalRead(MOTION_PIN);
  eMBPollWithWDT();
  update_led();
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  trigger_led();
  return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  trigger_led();
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
  trigger_led();
  return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  trigger_led();
  return MB_ENOREG;
}

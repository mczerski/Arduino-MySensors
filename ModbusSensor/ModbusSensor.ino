#include <mb_with_wdt.h>
#include <BME280I2C.h>
#include <ArtronShop_BH1750.h>

#define VERSION_MAJOR "1"
#define VERSION_MINOR "0"

#define TEST

#ifdef TEST
#define SLAVE_ID 0
#define SLAVE_NAME "Test"
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define MOTION
#define MOTION_PIN 3

#endif

#ifdef KITCHEN_MOTION
#define SLAVE_ID 4
#define SLAVE_NAME "Kitchen Motion"
#define MOTION
#define MOTION_PIN 2
#define FLASH_ID 0x1F65
#define LED_PIN A1
#endif

const char additional_info[] = SLAVE_NAME " v" VERSION_MAJOR "." VERSION_MINOR;
SPIFlash flash(8, FLASH_ID);
ArtronShop_BH1750 bh1750(0x23, &Wire);
BME280I2C bmeSensor(
  BME280I2C::Settings(
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::Mode_Forced,
    BME280::StandbyTime_1000ms,
    BME280::Filter_Off,
    BME280::SpiEnable_False,
    0x77
  )
);

struct InputRegisters_ {
  uint16_t motion_state;
  uint16_t luminance;
  uint16_t humidity;
  uint16_t temperature;
  uint16_t status;
};
typedef ModbusDataBlock<InputRegisters_> InputRegisters;
InputRegisters input_register;


void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, SLAVE_ID, additional_info, sizeof(additional_info) - 1, &flash, LED_PIN);
  #ifdef MOTION
  pinMode(MOTION_PIN, INPUT_PULLUP);
  #endif
  Wire.begin();
  input_register.s.status = 0;
  input_register.s.status |= int(bmeSensor.begin());
  input_register.s.status |= (int(bh1750.begin()) << 1);
}

void loop() {
  eMBPollWithWDT();
  input_register = {
    digitalRead(MOTION_PIN),
    round(bh1750.light()),
    round(bmeSensor.hum()),
    round(bmeSensor.temp() * 10),
    input_register.s.status
  };
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return eMBCopyToRegBuffer(input_register, pucRegBuffer, usAddress, usNRegs);
}

eMBErrorCode eMBRegHoldingCB(USHORT * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  return MB_ENOREG;
}

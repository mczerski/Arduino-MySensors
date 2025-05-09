#include <mb_with_wdt.h>
#include <BME280I2C.h>
#include <ArtronShop_BH1750.h>

#define VERSION_MAJOR "1"
#define VERSION_MINOR "2"

#define KITCHEN_MOTION

#ifdef TEST
#define SLAVE_ID 0
#define SLAVE_NAME "Test"
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define MOTION_SENSOR
#define MOTION_PIN 2
#define BH1750_SENSOR
#define BME280_SENSOR
#endif

#ifdef KITCHEN_MOTION
#define SLAVE_ID 4
#define SLAVE_NAME "Kitchen Motion"
#define FLASH_ID 0x1F65
#define LED_PIN A1
#define MOTION_SENSOR
#define MOTION_PIN 2
#define BH1750_SENSOR
#define BME280_SENSOR
#endif

const char additional_info[] = SLAVE_NAME " v" VERSION_MAJOR "." VERSION_MINOR;
SPIFlash flash(8, FLASH_ID);

#ifdef BH1750_SENSOR
ArtronShop_BH1750 bh1750(0x23, &Wire);
#endif

#ifdef BME280_SENSOR
BME280I2C bmeSensor(
  BME280I2C::Settings(
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::Mode_Normal,
    BME280::StandbyTime_1000ms,
    BME280::Filter_16,
    BME280::SpiEnable_False,
    0x77
  )
);
#endif

struct InputRegisters_ {
  uint16_t status;
  uint16_t motion_state;
  uint16_t luminance;
  uint16_t humidity;
  uint16_t temperature;
  uint16_t pressure;
};
typedef ModbusDataBlock<InputRegisters_> InputRegisters;
InputRegisters input_register = {0};


void setup() {
  eMBInitWithWDT(MB_RTU, 0, WDTO_8S, SLAVE_ID, additional_info, sizeof(additional_info) - 1, &flash, LED_PIN);
#ifdef MOTION_SENSOR
  pinMode(MOTION_PIN, INPUT_PULLUP);
#endif
  Wire.begin();
  input_register.s.status = 0;
#ifdef BH1750_SENSOR
  input_register.s.status |= (int(bh1750.begin()) << 1);
#endif
#ifdef BME280_SENSOR
  input_register.s.status |= int(bmeSensor.begin());
#endif
}

void loop() {
  eMBPollWithWDT();
#ifdef MOTION_SENSOR
  input_register.s.motion_state = digitalRead(MOTION_PIN);
#endif
#ifdef BH1750_SENSOR
  uint16_t luminance = round(bh1750.light());
  if (abs(luminance - input_register.s.luminance) > 3) input_register.s.luminance = luminance;
#endif
#ifdef BME280_SENSOR
  uint16_t humidity = round(bmeSensor.hum());
  uint16_t temperature = round(bmeSensor.temp() * 10);
  uint16_t pressure = round(bmeSensor.pres() / 10);
  if (abs(humidity - input_register.s.humidity) > 3) input_register.s.humidity = humidity;
  if (abs(temperature - input_register.s.temperature) > 5) input_register.s.temperature = temperature;
  if (abs(pressure - input_register.s.pressure) > 5) input_register.s.pressure = pressure;
#endif
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

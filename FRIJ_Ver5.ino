#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "Core.h"
#include "Utility.h"
#include "EEPROMFile.h"
#include "GSM.h"
#include "FRIJ_HVAC.h"
#include "FRIJ_DISPLAY.h"
#include "FRIJ_Bat_Temp.h"
#include "FRIJ_Sys_Parameter.h"
#include "FRIJ_GPIO.h"
#include "FRIJ_Operation.h"
#include "GPS.h"




void setup()
{
  system_time = millis();
  delay(100);
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);


#ifdef debug
  FRIJ.begin(BAUD_RATE);
  FRIJ.setTimeout(50);
#if NEW_FRIJ
  //while (!FRIJ);
#endif
#endif

  RTC_Setup();
  FRIJ_Bat_Temp_Setup();
  EEPROMSetup();
  FRIJ_GPIO_setup();
  FRIJ_HVAC_Setup();
  init_display();
  setUp_GSM();
  setup_GPS();
  pdTimer = millis();
  beatTimer = millis() + 10000;
}

void loop()
{
  FRIJ_Operation_Loop();
  synchronize_time();
  display_process();
  GSM_Process();
  FRIJ_GPIO_Loop();
  FRIJ_Bat_Temp_Loop();
  FRIJ_HVAC_Loop();
  //handleGetCoordinate();
}
